#pragma once

#include "storage_generic.hpp"
#include "identification/utility.hpp"

namespace utils::storage {
	namespace detail {
		struct storage_immutable : storage_generic {
			void init() override {
				this->storage_path_ =  get_program_data_path() / "storage.zprx";
				this->load();
			}

			~storage_immutable() override = default;

			std::expected<void, std::string> set(const std::string& key, const std::string& value) override {
				std::lock_guard lock(this->mutex);

				// immutable: once set, cannot be changed
				if (this->storage_data.contains(key)) {
					return std::unexpected("key already exists in immutable storage: " + key);
				}

				this->storage_data[key] = value;
				if (!this->save()) {
					return std::unexpected("failed to persist immutable storage to disk");
				}

				return {};
			}

			std::optional<std::string> get(const std::string& key) const override {
				std::lock_guard lock(this->mutex);

				auto it = this->storage_data.find(key);
				if (it == this->storage_data.end()) {
					return std::nullopt;
				}

				return it->second;
			}

			bool remove(const std::string& key) override {
				// immutable storage does not support removal
				return false;
			}

			bool contains(const std::string& key) const override {
				std::lock_guard lock(this->mutex);
				return this->storage_data.contains(key);
			}

			std::expected<void, std::string> clear() override {
				// immutable storage does not support clearing
				return std::unexpected("clearing operation not supported");
			}

			std::size_t size() const override {
				std::lock_guard lock(this->mutex);
				return this->storage_data.size();
			}

			std::vector<std::string> keys() const override {
				std::lock_guard lock(this->mutex);
				std::vector<std::string> result;

				result.reserve(this->storage_data.size());
				for (const auto& [key, _] : this->storage_data) {
					result.push_back(key);
				}

				return result;
			}

		private:
			std::filesystem::path storage_path_;
			std::map<std::string, std::string> storage_data;
			mutable std::mutex mutex;

			static std::vector<std::uint8_t> encrypt(const std::string& plaintext) {
				DATA_BLOB input{
					.cbData = VAL_AS(DWORD, plaintext.size()),
					.pbData = PTR_AS(std::uint8_t*, const_cast<char*>(plaintext.data()))
				};

				DATA_BLOB output{};
				if (!CryptProtectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_LOCAL_MACHINE, &output)) {
					return {};
				}

				std::vector<std::uint8_t> result(output.pbData, output.pbData + output.cbData);
				LocalFree(output.pbData);
				return result;
			}

			static std::optional<std::string> decrypt(const std::vector<std::uint8_t>& ciphertext) {
				DATA_BLOB input{
					.cbData = VAL_AS(DWORD, ciphertext.size()),
					.pbData = const_cast<std::uint8_t*>(ciphertext.data())
				};

				DATA_BLOB output{};
				if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr,CRYPTPROTECT_LOCAL_MACHINE, &output)) {
					return std::nullopt;
				}

				std::string result(PTR_AS(char*, output.pbData), output.cbData);
				LocalFree(output.pbData);
				return result;
			}

			bool load() {
				if (!std::filesystem::exists(this->storage_path_)) {
					return true;
				}

				std::ifstream file(this->storage_path_, std::ios::binary);
				if (!file.is_open()) {
					return false;
				}

				std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
				file.close();

				if (contents.empty()) {
					return true;
				}

				// file format: [std::uint32_t entry_count] then per entry:
				//   [std::uint32_t key_len][key_bytes][std::uint32_t val_blob_len][encrypted_val_bytes]
				const std::uint8_t* ptr = contents.data();
				const std::uint8_t* end = ptr + contents.size();

				auto read_u32 = [&]() -> std::optional<std::uint32_t> {
					if (ptr + sizeof(std::uint32_t) > end) return std::nullopt;
					std::uint32_t val;
					std::memcpy(&val, ptr, sizeof(std::uint32_t));
					ptr += sizeof(std::uint32_t);
					return val;
				};

				auto entry_count = read_u32();
				if (!entry_count) {
					return false;
				}

				for (std::uint32_t i = 0; i < entry_count.value(); ++i) {
					auto key_len = read_u32();
					if (!key_len || ptr + key_len.value() > end) {
						return false;
					}

					std::string key(PTR_AS(const char*, ptr), key_len.value());
					ptr += key_len.value();

					auto blob_len = read_u32();
					if (!blob_len || ptr + blob_len.value() > end) {
						return false;
					}

					std::vector<std::uint8_t> blob(ptr, ptr + blob_len.value());
					ptr += blob_len.value();

					auto decrypted = this->decrypt(blob);
					if (!decrypted) {
						return false;
					}

					storage_data[std::move(key)] = std::move(decrypted.value());
				}

				return true;
			}

			bool save() const {
				std::filesystem::create_directories(this->storage_path_.parent_path());

				std::ofstream file(this->storage_path_, std::ios::binary | std::ios::trunc);
				if (!file.is_open()) {
					return false;
				}

				auto write_u32 = [&](std::uint32_t val) {
					file.write(PTR_AS(const char*, &val), sizeof(std::uint32_t));
				};

				write_u32(VAL_AS(std::uint32_t, storage_data.size()));

				for (const auto& [key, value] : storage_data) {
					write_u32(VAL_AS(std::uint32_t, key.size()));
					file.write(key.data(), key.size());

					auto encrypted = this->encrypt(value);
					write_u32(VAL_AS(std::uint32_t, encrypted.size()));
					file.write(PTR_AS(const char*, encrypted.data()), encrypted.size());
				}

				return file.good();
			}
		};
	}

	inline detail::storage_immutable immutable_storage;
}
