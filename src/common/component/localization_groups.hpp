#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace localized_strings {
	class override_groups {
	public:
		using translations = std::unordered_map<std::string, std::string>;

		void replace(const std::string& owner, const translations& values) {
			translations normalized;
			for (const auto& [key, text] : values) {
				normalized[normalize(key)] = text;
			}

			auto& current = groups_[owner];
			if (current && *current == normalized) {
				return;
			}

			retained_.push_back(std::make_unique<const translations>(std::move(normalized)));
			current = retained_.back().get();
		}

		const char* find(std::string_view key) const {
			const auto normalized = normalize(key);
			for (const auto& [owner, values] : groups_) {
				if (!values) {
					continue;
				}

				if (const auto it = values->find(normalized); it != values->end()) {
					return it->second.c_str();
				}
			}

			return nullptr;
		}

	private:
		static std::string normalize(std::string_view key) {
			std::string result(key);
			for (auto& character : result) {
				if (character >= 'A' && character <= 'Z') {
					character += 'a' - 'A';
				}
			}

			return result;
		}

		std::map<std::string, const translations*> groups_;
		std::vector<std::unique_ptr<const translations>> retained_;
	};
}