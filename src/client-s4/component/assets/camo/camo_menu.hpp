#pragma once

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace assets::camo::menu {
	struct table_entry {
		std::string name;
		std::optional<std::uint32_t> menu_id;
		std::string category;
		std::string localization;
		std::string icon;
	};

	inline std::optional<std::uint32_t> numeric_id(std::string_view text) {
		if (text.empty()) {
			return {};
		}

		std::uint32_t value{};
		const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
		if (error != std::errc{} || end != text.data() + text.size()) {
			return {};
		}

		return value;
	}

	inline std::optional<table_entry> parse_row(std::span<const std::string> cells) {
		// asset name, category, localization, icon
		if (cells.size() < 8 || cells[1].empty()) {
			return {};
		}

		const auto id = numeric_id(cells[0]);
		if (!id) {
			return {};
		}

		return table_entry{ cells[1], id, cells[3], cells[7], cells.size() > 8 ? cells[8] : std::string{} };
	}

	struct customization {
		std::optional<std::string> display_name;
		std::filesystem::path icon_file;
	};

	struct overrides {
		std::unordered_map<std::string, std::string> names;
		std::unordered_map<std::string, std::filesystem::path> icons;
	};

	inline overrides resolve(std::span<const table_entry> entries, const std::map<std::string, customization>& customizations,
		std::vector<std::string>& errors) {
		overrides result;
		std::map<std::string, table_entry> sorted;
		for (const auto& entry : entries) {
			sorted.try_emplace(entry.name, entry);
		}

		for (const auto& [name, entry] : sorted) {
			const auto it = customizations.find(name);
			if (it == customizations.end()) {
				continue;
			}

			const auto& custom = it->second;
			if (custom.display_name && !entry.localization.empty()) {
				const auto [value, inserted] = result.names.emplace(entry.localization, *custom.display_name);
				if (!inserted && value->second != *custom.display_name) {
					errors.emplace_back("Shared localization '" + entry.localization + "': keeping the first camos custom name..");
				}
			}

			if (!custom.icon_file.empty() && !entry.icon.empty()) {
				const auto [value, inserted] = result.icons.emplace(entry.icon, custom.icon_file);
				if (!inserted && value->second != custom.icon_file) {
					errors.emplace_back("Shared icon '" + entry.icon + "': keeping the first camos custom icon..");
				}
			}
		}

		return result;
	}
}
