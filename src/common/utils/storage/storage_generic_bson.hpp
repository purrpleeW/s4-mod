#pragma once

#include "storage_generic.hpp"

namespace utils::storage::detail {
	struct storage_generic_bson : storage_generic {
        ~storage_generic_bson() override = default;

        std::expected<void, std::string> set(const std::string& key, const std::string& value) override {
            std::lock_guard lock(this->mutex_);
            this->data_[key] = value;

            if (!this->save()) {
                return std::unexpected("failed to persist bson storage to disk");
            }
            return {};
        }

        std::optional<std::string> get(const std::string& key) const override {
            std::lock_guard lock(this->mutex_);
            auto it = this->data_.find(key);
            if (it == this->data_.end()) {
                return std::nullopt;
            }
            return it->second;
        }

        bool remove(const std::string& key) override {
            std::lock_guard lock(this->mutex_);
            if (this->data_.erase(key) == 0) {
                return false;
            }

            return this->save();
        }

        bool contains(const std::string& key) const override {
            std::lock_guard lock(this->mutex_);
            return this->data_.contains(key);
        }

        std::expected<void, std::string> clear() override {
            std::lock_guard lock(this->mutex_);
            this->data_.clear();

			if (auto save_err = this->save(); !save_err) {
				return std::unexpected("failed to save cleared out storage");
			}

			return {};
        }

        std::size_t size() const override {
            std::lock_guard lock(this->mutex_);
            return this->data_.size();
        }

        std::vector<std::string> keys() const override {
            std::lock_guard lock(this->mutex_);
            std::vector<std::string> result;
            result.reserve(this->data_.size());
            for (const auto& [key, _] : this->data_) {
                result.push_back(key);
            }
            return result;
        }

	protected:
        std::filesystem::path storage_path_;
        std::map<std::string, std::string> data_;
        mutable std::mutex mutex_;

        bool load() {
            if (!std::filesystem::exists(this->storage_path_)) {
                return true;
            }

            std::ifstream file(this->storage_path_, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            std::vector<uint8_t> contents((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
            file.close();

            if (contents.empty()) {
                return true;
            }

            try {
                auto j = nlohmann::json::from_bson(contents);
                for (auto& [key, value] : j.items()) {
                    if (value.is_string()) {
                        this->data_[key] = value.get<std::string>();
                    }
                }
            } catch (const nlohmann::json::exception&) {
                return false;
            }

            return true;
        }

        bool save() const {
            std::filesystem::create_directories(this->storage_path_.parent_path());

            std::ofstream file(this->storage_path_, std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                return false;
            }

            nlohmann::json j(this->data_);
            auto bson = nlohmann::json::to_bson(j);
            file.write(PTR_AS(const char*, bson.data()), bson.size());
            return file.good();
        }
    };
}
