#pragma once
#include "common_core.hpp"

namespace localized_strings {
	class original_localization {
	public:
		original_localization(const std::string& translation_key, const std::string& text)
			: translation_key_(translation_key)
			, text_(text)
		{}

		std::string translation_key_;
		std::string text_;
	};

	enum class global_translate_mode {
		normal,
		translated_with_key,
		not_translated
	};

	std::string get(const std::string& key);
	void override(const std::regex& exp, const std::function<std::string(const original_localization&, const std::smatch&)>& callback);
	void redirect(const std::string& key, const std::string& new_key, const std::function<bool()>& condition);

	void set(const std::string& key, const std::string& value);
	void unset(const std::string& key);

	inline void override(const std::regex& exp, const std::string& value) {
		override(exp, [value](const original_localization&, const std::smatch&) {
			return value;
		});
	}

	inline void override(const std::string& key, const std::function<std::string(const original_localization&)>& callback) {
		static const std::regex meta_characters(R"([.^$|()\\[\]{}*+?])");
		std::regex pattern(std::regex_replace(key, meta_characters, R"(\$&)"));
		override(pattern, [callback](const original_localization& original, const std::smatch&) {
			return callback(original);
		});
	}

	inline void override(const std::string& key, const std::string& value) {
		override(key, [value](const original_localization&) {
			return value;
		});
	}

	inline void redirect(const std::string& key, const std::string& new_key) {
		redirect(key, new_key, [] {
			return true;
		});
	}
}
