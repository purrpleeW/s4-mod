#include "common.hpp"
#include "component/assets/camo/asset_camo_ui.hpp"

#include "component/assets.hpp"
#include "component/assets/asset_image.hpp"
#include "component/assets/asset_read.hpp"
#include "component/assets/camo/camo_settings.hpp"

#include <component/localized_strings.hpp>

namespace assets::camo::ui {
	namespace {
		std::vector<menu::table_entry> read_menu_entries() {
			std::vector<menu::table_entry> entries;

			for (const auto& row : reader::read_table("mp/camotable.csv")) {
				const auto entry = menu::parse_row(row);
				if (!entry) {
					continue;
				}

				entries.push_back(*entry);
			}

			return entries;
		}

		menu::customization read_customization(const std::string& name) {
			const auto directory = assets::get_content_path("camo", name);
			menu::customization custom;
			std::vector<std::string> errors;
			if (const auto document = reader::read_json(directory / "properties.json")) {
				custom.display_name = settings::display_name(*document, errors);
			}

			image::find_image_file(directory / "icon", custom.icon_file);
			for (const auto& error : errors) {
				LOG("Component/Assets/Camo", WARN, "'{}': {}", name, error);
			}

			return custom;
		}
	}

	menu::overrides refresh(const std::unordered_set<std::string>& folders) {
		const auto entries = read_menu_entries();
		std::map<std::string, menu::customization> customizations;

		for (const auto& name : folders) {
			customizations.emplace(name, read_customization(name));
		}

		std::vector<std::string> errors;
		auto result = menu::resolve(entries, customizations, errors);
		for (const auto& error : errors) {
			LOG("Component/Assets/Camo", WARN, "{}", error);
		}

		localized_strings::set_group("s4-custom-camos", result.names);
		if (!entries.empty()) {
			LOG("Component/Assets/Camo", INFO, "Resolved {} custom name(s), and {} custom icon(s)!", result.names.size(), result.icons.size());
		}
		else if (!folders.empty()) {
			LOG("Component/Assets/Camo", INFO, "Camo menu waiting for mp/camotable.csv");
		}

		return result;
	}
}
