#include "safe_release.hpp"
#include "string.hpp"
#include "cryptography.hpp"

#include <cstdint>
#include <random>
#include <fstream>

#include "nt.hpp"
#include "tomcrypt.h"
#include "safe_release.hpp"

#undef max
using namespace std::string_literals;

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace utils::cryptography {
	namespace {
		[[gnu::constructor]] void ltc_startup() {
			ltc_mp = ltm_desc;
			register_all_ciphers();
			register_all_hashes();
			register_all_prngs();
		}

		class prng {
		public:
			prng(const ltc_prng_descriptor& descriptor, const bool autoseed = true)
				: state_(std::make_unique<prng_state>())
				, descriptor_(descriptor)
			{
				this->id_ = register_prng(&descriptor);
				if (this->id_ == -1) {
					throw std::runtime_error("PRNG "s + this->descriptor_.name + " could not be registered!");
				}

				if (autoseed) {
					this->auto_seed();
				}
				else {
					this->descriptor_.start(this->state_.get());
				}
			}

			~prng() {
				this->descriptor_.done(this->state_.get());
			}

			prng_state* get_state() const {
				this->descriptor_.ready(this->state_.get());
				return this->state_.get();
			}

			int get_id() const {
				return this->id_;
			}

			void add_entropy(const void* data, const std::size_t length) const {
				this->descriptor_.add_entropy(VAL_AS(const std::uint8_t*, data), length, this->state_.get());
			}

			void read(void* data, const std::size_t length) const {
				this->descriptor_.read(VAL_AS(unsigned char*, data), length, this->get_state());
			}

		private:
			int id_;
			std::unique_ptr<prng_state> state_;
			const ltc_prng_descriptor& descriptor_;

			void auto_seed() const {
				rng_make_prng(128, this->id_, this->state_.get(), nullptr);

				int i[4]; // uninitialized data
				auto* i_ptr = &i;
				this->add_entropy(&i, sizeof(i));
				this->add_entropy(&i_ptr, sizeof(int*));

				auto t = time(nullptr);
				this->add_entropy(&t, sizeof(t));

				std::random_device rd{};
				for (auto j = 0; j < 4; ++j) {
					const auto x = rd();
					this->add_entropy(&x, sizeof(x));
				}
			}
		};

		const prng prng_(fortuna_desc);
	}

	ecc::key::key() {
		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	ecc::key::~key() {
		this->free();
	}

	ecc::key::key(key&& obj) noexcept
		: key()
	{
		this->operator=(std::move(obj));
	}

	ecc::key::key(const key& obj)
		: key()
	{
		this->operator=(obj);
	}

	ecc::key& ecc::key::operator=(key&& obj) noexcept {
		if (this != &obj) {
			std::memmove(&this->key_storage_, &obj.key_storage_, sizeof(this->key_storage_));
			ZeroMemory(&obj.key_storage_, sizeof(obj.key_storage_));
		}

		return *this;
	}

	ecc::key& ecc::key::operator=(const key& obj) {
		if (this != &obj && obj.is_valid()) {
			this->deserialize(obj.serialize(obj.key_storage_.type));
		}

		return *this;
	}

	bool ecc::key::is_valid() const {
		return (!memory::is_set(&this->key_storage_, 0, sizeof(this->key_storage_)));
	}

	ecc_key& ecc::key::get() {
		return this->key_storage_;
	}

	const ecc_key& ecc::key::get() const {
		return this->key_storage_;
	}

	std::string ecc::key::get_public_key() const {
		std::uint8_t buffer[512] = {0};
		unsigned long length = sizeof(buffer);

		if (ecc_ansi_x963_export(&this->key_storage_, buffer, &length) == CRYPT_OK) {
			return std::string(PTR_AS(const char*, buffer), length);
		}

		return {};
	}

	void ecc::key::set(const std::string& pub_key_buffer) {
		this->free();

		if (ecc_ansi_x963_import(PTR_AS(const std::uint8_t*, pub_key_buffer.data()), pub_key_buffer.size(), &this->key_storage_) != CRYPT_OK) {
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	void ecc::key::deserialize(const std::string& key) {
		this->free();

		if (ecc_import(PTR_AS(const std::uint8_t*, key.data()), key.size(), &this->key_storage_) != CRYPT_OK) {
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	std::string ecc::key::serialize(const int type) const {
		std::uint8_t buffer[4096] = { 0 };
		unsigned long length = sizeof(buffer);

		if (ecc_export(buffer, &length, type, &this->key_storage_) == CRYPT_OK) {
			return { PTR_AS(const char*, buffer), length };
		}

		return {};
	}

	std::string ecc::key::get_openssl() const {
		std::uint8_t buffer[4096] = { 0 };
		unsigned long length = sizeof(buffer);

		if (ecc_export_openssl(buffer, &length, PK_PUBLIC, &this->key_storage_) == CRYPT_OK) {
			return { PTR_AS(const char*, buffer), length };
		}

		return {};
	}

	void ecc::key::set_openssl(const std::string& key) {
		this->free();

		if (ecc_import_openssl(PTR_AS(const std::uint8_t*, key.data()), key.size(), &this->key_storage_) != CRYPT_OK) {
			ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
		}
	}

	void ecc::key::free() {
		if (this->is_valid()) {
			ecc_free(&this->key_storage_);
		}

		ZeroMemory(&this->key_storage_, sizeof(this->key_storage_));
	}

	bool ecc::key::operator==(key& key) const {
		return (this->is_valid() && key.is_valid() && this->serialize(PK_PUBLIC) == key.serialize(PK_PUBLIC));
	}

	std::uint64_t ecc::key::get_hash() const {
		const auto hash = sha1::compute(this->get_public_key());
		if (hash.size() >= 8) {
			return DEREF_PTR_AS(const std::uint64_t, hash.data());
		}

		return 0;
	}

	ecc::key ecc::generate_key(const int bits) {
		key key;
		ecc_make_key(prng_.get_state(), prng_.get_id(), bits / 8, &key.get());

		return key;
	}

	ecc::key ecc::generate_key(const int bits, const std::string& entropy) {
		key key{};
		const prng yarrow(yarrow_desc, false);
		yarrow.add_entropy(entropy.data(), entropy.size());

		ecc_make_key(yarrow.get_state(), yarrow.get_id(), bits / 8, &key.get());

		return key;
	}

	std::string ecc::sign_message(const key& key, const std::string& message) {
		if (!key.is_valid()) {
			return {};
		}

		std::uint8_t buffer[512];
		unsigned long length = sizeof(buffer);

		const auto hash = sha512::compute(message);

		ecc_sign_hash(PTR_AS(const std::uint8_t*, hash.data()), hash.size(), buffer, &length, prng_.get_state(), prng_.get_id(), &key.get());

		return std::string(PTR_AS(const char*, buffer), length);
	}

	bool ecc::verify_message(const key& key, const std::string& message, const std::string& signature) {
		if (!key.is_valid()) {
			return false;
		}

		const auto hash = sha512::compute(message);

		auto result = 0;
		return (ecc_verify_hash(PTR_AS(const std::uint8_t*, signature.data()), signature.size(),
			PTR_AS(const std::uint8_t*, hash.data()), hash.size(), &result, &key.get()
		) == CRYPT_OK && result != 0);
	}

	bool ecc::encrypt(const key& key, std::string& data) {
		std::string out_data{};
		out_data.resize(std::max(data.size() * 3, 0x100ull));

		auto out_len = VAL_AS(unsigned long, out_data.size());
		auto crypt = [&]() {
			return ecc_encrypt_key(PTR_AS(const std::uint8_t*, data.data()),
				data.size(), PTR_AS(std::uint8_t*, out_data.data()), &out_len,
				prng_.get_state(), prng_.get_id(), find_hash("sha512"), &key.get()
			);
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW) {
			out_data.resize(out_len);
			res = crypt();
		}

		if (res != CRYPT_OK) {
			return false;
		}

		out_data.resize(out_len);
		data = std::move(out_data);
		return true;
	}

	bool ecc::decrypt(const key& key, std::string& data) {
		std::string out_data{};
		out_data.resize(std::max(data.size() * 3, 0x100ull));

		auto out_len = VAL_AS(unsigned long, out_data.size());
		auto crypt = [&]() {
			return ecc_decrypt_key(PTR_AS(const std::uint8_t*, data.data()), data.size(),
				PTR_AS(std::uint8_t*, out_data.data()), &out_len, &key.get()
			);
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW) {
			out_data.resize(out_len);
			res = crypt();
		}

		if (res != CRYPT_OK) {
			return false;
		}

		out_data.resize(out_len);
		data = std::move(out_data);
		return true;
	}

	std::string rsa::encrypt(const std::string& data, const std::string& hash, const std::string& key) {
		rsa_key new_key;
		rsa_import(PTR_AS(const std::uint8_t*, key.data()), key.size(), &new_key);
		const auto _ = safe_release([&new_key]{
			rsa_free(&new_key);
		});

		std::string out_data{};
		out_data.resize(std::max(data.size() * 3, 0x100ull));

		auto out_len = VAL_AS(unsigned long, out_data.size());
		auto crypt = [&]() {
			return rsa_encrypt_key(PTR_AS(const std::uint8_t*, data.data()), data.size(),
				PTR_AS(std::uint8_t*, out_data.data()), &out_len,
				PTR_AS(const std::uint8_t*, hash.data()), hash.size(),
				prng_.get_state(), prng_.get_id(), find_hash("sha512"), &new_key
			);
		};

		auto res = crypt();

		if (res == CRYPT_BUFFER_OVERFLOW) {
			out_data.resize(out_len);
			res = crypt();
		}

		if (res == CRYPT_OK) {
			out_data.resize(out_len);
			return out_data;
		}

		return {};
	}

	std::string des3::encrypt(const std::string& data, const std::string& iv, const std::string& key) {
		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, PTR_AS(const std::uint8_t*, iv.data()), PTR_AS(const std::uint8_t*, key.data()), VAL_AS(int, key.size()), 0, &cbc);
		cbc_encrypt(PTR_AS(const std::uint8_t*, data.data()), PTR_AS(std::uint8_t*, enc_data.data()), data.size(), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string des3::decrypt(const std::string& data, const std::string& iv, const std::string& key) {
		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto des3 = find_cipher("3des");

		cbc_start(des3, PTR_AS(const std::uint8_t*, iv.data()), PTR_AS(const std::uint8_t*, key.data()), VAL_AS(int, key.size()), 0, &cbc);
		cbc_decrypt(PTR_AS(const std::uint8_t*, data.data()), PTR_AS(std::uint8_t*, dec_data.data()), data.size(), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	std::string tiger::compute(const std::string& data, const bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), data.size(), hex);
	}

	std::string tiger::compute(const std::uint8_t* data, const std::size_t length, const bool hex) {
		std::uint8_t buffer[24] = {0};

		hash_state state;
		tiger_init(&state);
		tiger_process(&state, data, length);
		tiger_done(&state, buffer);

		std::string hash(PTR_AS(const char*, buffer), sizeof(buffer));
		if (!hex) {
			return hash;
		}

		return string::dump_hex(hash, "");
	}

	std::string aes::encrypt(const std::string& data, const std::string& iv, const std::string& key) {
		std::string enc_data;
		enc_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, PTR_AS(const std::uint8_t*, iv.data()), PTR_AS(const std::uint8_t*, key.data()), VAL_AS(int, key.size()), 0, &cbc);
		cbc_encrypt(PTR_AS(const std::uint8_t*, data.data()), PTR_AS(std::uint8_t*, enc_data.data()), data.size(), &cbc);
		cbc_done(&cbc);

		return enc_data;
	}

	std::string aes::decrypt(const std::string& data, const std::string& iv, const std::string& key) {
		std::string dec_data;
		dec_data.resize(data.size());

		symmetric_CBC cbc;
		const auto aes = find_cipher("aes");

		cbc_start(aes, PTR_AS(const std::uint8_t*, iv.data()), PTR_AS(const std::uint8_t*, key.data()), VAL_AS(int, key.size()), 0, &cbc);
		cbc_decrypt(PTR_AS(const std::uint8_t*, data.data()), PTR_AS(std::uint8_t*, dec_data.data()), data.size(), &cbc);
		cbc_done(&cbc);

		return dec_data;
	}

	std::string hmac_sha1::compute(const std::string& data, const std::string& key) {
		std::string buffer;
		buffer.resize(20);

		hmac_state state;
		hmac_init(&state, find_hash("sha1"), PTR_AS(const std::uint8_t*, key.data()), key.size());
		hmac_process(&state, PTR_AS(const std::uint8_t*, data.data()), VAL_AS(int, data.size()));

		auto out_len = VAL_AS(unsigned long, buffer.size());
		hmac_done(&state, PTR_AS(std::uint8_t*, buffer.data()), &out_len);

		buffer.resize(out_len);
		return buffer;
	}

	std::string sha1::compute(const std::string& data, const bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), data.size(), hex);
	}

	std::string sha1::compute(const std::uint8_t* data, const std::size_t length, const bool hex) {
		std::uint8_t buffer[20] = { 0 };

		hash_state state;
		sha1_init(&state);
		sha1_process(&state, data, length);
		sha1_done(&state, buffer);

		std::string hash(PTR_AS(const char*, buffer), sizeof(buffer));
		if (!hex) {
			return hash;
		}

		return string::dump_hex(hash, "");
	}

	std::string sha256::compute(const std::string& data, const bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), data.size(), hex);
	}

	std::string sha256::compute(const std::uint8_t* data, const std::size_t length, const bool hex) {
		std::uint8_t buffer[32] = { 0 };

		hash_state state;
		sha256_init(&state);
		sha256_process(&state, data, length);
		sha256_done(&state, buffer);

		std::string hash(PTR_AS(const char*, buffer), sizeof(buffer));
		if (!hex) {
			return hash;
		}

		return string::dump_hex(hash, "");
	}

	std::string sha512::compute(const std::string& data, const bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), data.size(), hex);
	}

	std::string sha512::compute(const std::uint8_t* data, const std::size_t length, const bool hex) {
		std::uint8_t buffer[64] = { 0 };

		hash_state state;
		sha512_init(&state);
		sha512_process(&state, data, length);
		sha512_done(&state, buffer);

		std::string hash(PTR_AS(const char*, buffer), sizeof(buffer));
		if (!hex) {
			return hash;
		}

		return string::dump_hex(hash, "");
	}

	std::string md5::compute(const std::string& data, const bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), data.size(), hex);
	}

	std::string md5::compute(const std::uint8_t* data, const std::size_t length, const bool hex) {
		std::uint8_t buffer[16] = { 0 };

		hash_state state;
		md5_init(&state);
		md5_process(&state, data, length);
		md5_done(&state, buffer);

		std::string hash(PTR_AS(const char*, buffer), sizeof(buffer));
		if (!hex) {
			return hash;
		}

		return string::dump_hex(hash, "");
	}

	std::string base64::encode(const std::uint8_t* data, const std::size_t len) {
		std::string result;
		result.resize((len + 2) * 2);

		auto out_len = VAL_AS(unsigned long, result.size());
		if (base64_encode(data, len, result.data(), &out_len) != CRYPT_OK) {
			return {};
		}

		result.resize(out_len);
		return result;
	}

	std::string base64::encode(const std::string& data) {
		return encode(PTR_AS(const std::uint8_t*, data.data()), VAL_AS(unsigned, data.size()));
	}

	std::string base64::decode(const std::string& data) {
		std::string result;
		result.resize((data.size() + 2) * 2);

		auto out_len = VAL_AS(unsigned long, result.size());
		if (base64_decode(data.data(), data.size(), PTR_AS(std::uint8_t*, result.data()), &out_len) != CRYPT_OK) {
			return {};
		}

		result.resize(out_len);
		return result;
	}

	std::string crc32::compute(const std::uint8_t* data, std::size_t length, bool hex) {
		crc32_state md;
		unsigned char out[4]; // CRC32 = 32 bits (4 bytes)
		std::uint32_t crc = 0;

		// Initialize, process, and finalize
		crc32_init(&md);
		crc32_update(&md, data, VAL_AS(unsigned long, length));
		crc32_finish(&md, out, sizeof(uint32_t));

		// Convert result from 4 bytes (big endian) to uint32_t
		crc = (VAL_AS(std::uint32_t, out[0]) << 24) |
			(VAL_AS(std::uint32_t, out[1]) << 16) |
			(VAL_AS(std::uint32_t, out[2]) << 8)  |
			(VAL_AS(std::uint32_t, out[3]));

		// Format output
		std::ostringstream oss;
		if (hex) {
			oss << std::hex << std::setw(8) << std::setfill('0') << crc;
		}
		else {
			oss << crc;
		}

		return oss.str();
	}

	std::string crc32::compute(const std::string& data, bool hex) {
		return compute(PTR_AS(const std::uint8_t*, data.data()), VAL_AS(unsigned, data.size()), hex);
	}

	std::uint32_t jenkins_one_at_a_time::compute(const std::string& data) {
		return compute(data.data(), data.size());
	}

	std::uint32_t jenkins_one_at_a_time::compute(const char* key, const std::size_t len) {
		std::uint32_t hash, i;
		for (hash = i = 0; i < len; ++i) {
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	std::uint32_t platform_usernames::compute(const std::string& data) {
		return compute(data.data(), data.size());
	}

	std::uint32_t platform_usernames::compute(const char* key, const std::size_t len) {
		std::uint32_t hash = 0;

		if (!key) {
			return hash;
		}

		for (std::size_t i = 0; i < len && key[i] != '\0'; ++i) {
			hash = hash * 31 + VAL_AS(std::uint8_t, key[i]);
		}

		return hash;
	}

	std::uint32_t random::get_integer() {
		std::uint32_t result;
		random::get_data(&result, sizeof(result));
		return result;
	}

	std::uint64_t random::get_big_integer() {
		std::uint64_t result;
		random::get_data(&result, sizeof(result));
		return result;
	}

	std::string random::get_challenge() {
		std::string result;
		result.resize(sizeof(std::uint64_t));
		random::get_data(result.data(), result.size());
		return string::dump_hex(result, "");
	}

	void random::get_data(void* data, const std::size_t size) {
		prng_.read(data, size);
	}
}
