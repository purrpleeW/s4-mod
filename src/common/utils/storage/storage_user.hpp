#pragma once

#include "storage_generic.hpp"
#include "identification/utility.hpp"

namespace utils::storage {
	namespace detail {
		struct storage_user : storage_generic {
			void init() override {
				this->storage_path_ = get_documents_path() / "storage.ini";
				std::filesystem::create_directories(this->storage_path_.parent_path());
			}

			explicit storage_user(const std::string& section = "Settings")
				: section_(section)
			{}

			~storage_user() override = default;

			static storage_user get_section(const std::string& section) {
				return storage_user(section);
			}

			std::expected<void, std::string> set(const std::string& key, const std::string& value) override {
				std::lock_guard lock(this->mutex_);

				if (!WritePrivateProfileStringA(this->section_.c_str(),key.c_str(),value.c_str(),this->path_str().c_str())) {
					return std::unexpected("failed to write ini key: " + key + " (error: " + std::to_string(GetLastError()) + ")");
				}

				return {};
			}

			std::optional<std::string> get(const std::string& key) const override {
				std::lock_guard lock(this->mutex_);

				// start with a reasonable buffer, grow if needed
				DWORD buf_size = 256;
				std::string buffer;

				while (true) {
					buffer.resize(buf_size);

					auto result = GetPrivateProfileStringA(this->section_.c_str(), key.c_str(),
						nullptr, buffer.data(), buf_size, this->path_str().c_str()
					);

					if (result == 0) {
						return std::nullopt;
					}

					if (result == buf_size - 1) {
						buf_size *= 2;
						continue;
					}

					buffer.resize(result);
					return buffer;
				}
			}

			bool remove(const std::string& key) override {
				std::lock_guard lock(this->mutex_);

				if (!this->contains(key)) {
					return false;
				}

				return WritePrivateProfileStringA(this->section_.c_str(), key.c_str(), nullptr, this->path_str().c_str()) != 0;
			}

			bool contains(const std::string& key) const override {
				std::lock_guard lock(this->mutex_);

				// use a sentinel default value to detect absence
				constexpr const char* sentinel = "\x01\x02__MISSING__\x02\x01";

				char buffer[2]{};
				auto result = GetPrivateProfileStringA(this->section_.c_str(), key.c_str(),
					sentinel, buffer, sizeof(buffer), this->path_str().c_str()
				);

				// if the API returned our sentinel, the key doesn't exist
				// since sentinel is longer than buffer, result will be sizeof(buffer)-1
				// and buffer will start with sentinel's first char
				// for a proper check, use a larger buffer
				std::string check_buf(128, '\0');
				result = GetPrivateProfileStringA(this->section_.c_str(), key.c_str(), sentinel,
					check_buf.data(), VAL_AS(DWORD, check_buf.size()), this->path_str().c_str()
				);

				check_buf.resize(result);
				return check_buf != sentinel;
			}

			std::expected<void, std::string> clear() override {
				std::lock_guard lock(this->mutex_);

				auto success = WritePrivateProfileStringA(this->section_.c_str(), nullptr, nullptr, this->path_str().c_str()) != 0;
				if (success) {
					return {};
				}

				return std::unexpected("failed to clear user storage data (error: " + std::to_string(GetLastError()) + ")");
			}

			std::size_t size() const override {
				std::lock_guard lock(this->mutex_);
				return this->keys().size();
			}

			std::vector<std::string> keys() const override {
				std::lock_guard lock(this->mutex_);

				// passing nullptr as key returns all key names as double-null terminated list
				DWORD buf_size = 1024;
				std::vector<char> buffer;

				while (true) {
					buffer.resize(buf_size);

					auto result = GetPrivateProfileStringA(this->section_.c_str(), nullptr,
						nullptr, buffer.data(), buf_size, this->path_str().c_str()
					);

					if (result == 0) {
						return {};
					}

					if (result == buf_size - 2) {
						buf_size *= 2;
						continue;
					}

					break;
				}

				// parse double-null terminated list
				std::vector<std::string> result;
				const char* ptr = buffer.data();
				while (*ptr != '\0') {
					std::string key(ptr);
					result.push_back(std::move(key));
					ptr += result.back().size() + 1;
				}

				return result;
			}

		private:
			std::filesystem::path storage_path_;
			std::string section_;
			mutable std::mutex mutex_;
    		mutable std::once_flag init_flag_;

			void ensure_init() const {
				std::call_once(init_flag_, [this]() {
					// const_cast because init() is non-const, but we need
					// to call this from const methods like get()/contains()
					auto* self = const_cast<storage_user*>(this);
					self->storage_path_ = get_documents_path() / "storage.ini";
					std::filesystem::create_directories(self->storage_path_.parent_path());
				});
			}

			std::string path_str() const {
				this->ensure_init();
				return this->storage_path_.string();
			}
		};
	}

	inline detail::storage_user user_storage;
}