#include "common.hpp"
#include "component/assets.hpp"
#include "component/command.hpp"
#include "component/filesystem.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <component/localized_strings.hpp>
#include <component/localization_groups.hpp>
#include <loader/component_loader.hpp>
#include <utils/concurrency.hpp>
#include <utils/string.hpp>

#include <rapidjson/document.h>

namespace localized_strings {
	namespace {
		/*
			minor issue:
				localization dumping and loading sometimes crashes and bugs out if done potentially too early?
				i'm not exactly sure why but I'll look into it at some point. for now, just don't do it too early in the loading process and
		   it should be fine. tested on builds 1.07.1, 1.14, 1.16 and 1.24.1

				~ purrplee
		*/

		utils::concurrency::container<std::unordered_map<std::string, std::string>> direct_overrides;
		utils::concurrency::container<override_groups> grouped_overrides;

		std::map<std::string, std::string> stored_translations{};
		std::map<std::string, s4::LocalizeEntry*> stored_localize_additions{};

		const char* localize_string(const char* key) {
			if (!key[1]) {
				if ((*key & 0x80) != 0) {
					return "t";
				}

				return key;
			}

			const auto lower_key = utils::string::to_lower(key);

			auto found = direct_overrides.access<bool>([&](const std::unordered_map<std::string, std::string>& map) {
				const auto it = map.find(lower_key);
				if (it != map.end()) {
					stored_translations[lower_key] = it->second;
					return true;
				}

				return false;
			});

			if (found) {
				return stored_translations[lower_key].c_str();
			}

			const auto grouped = grouped_overrides.access<const char*>([&](const override_groups& groups) {
				return groups.find(lower_key);
			});

			if (grouped) {
				return grouped;
			}

			auto localize_entry = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_LOCALIZE, key, FALSE).localize_;
			return localize_entry && localize_entry->value_ ? localize_entry->value_ : key;
		}

		void init_locals() {
			auto localize_json = filesystem::read_file("assets/localize.json");

			rapidjson::Document doc;
			doc.Parse(localize_json.c_str());

			if (!doc.Empty()) {
				std::size_t loc_count = 0;

				for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
					if (!it->name.IsString() || !it->value.IsString()) {
						continue;
					}

					const std::string key = it->name.GetString();
					const std::string value = it->value.GetString();

					set(key, value);
					++loc_count;
				}

				LOG("Component/Localization", INFO, "Initialized {} localization{}.", loc_count, loc_count == 1 ? "" : "s");
			}
			else {
				LOG("Component/Localization", WARN, "Failed to load localization list.");
			}
		}
	}

	void set(const std::string& key, const std::string& value) {
		direct_overrides.access([&](std::unordered_map<std::string, std::string>& map) {
			map[utils::string::to_lower(key)] = value;
		});
	}

	void unset(const std::string& key) {
		direct_overrides.access([&](std::unordered_map<std::string, std::string>& map) {
			map.erase(utils::string::to_lower(key));
		});
	}

	void set_group(const std::string& owner, const std::unordered_map<std::string, std::string>& values) {
		grouped_overrides.access([&](override_groups& groups) {
			groups.replace(owner, values);
		});
	}

	struct component final : generic_component {
		void post_load() override {
			filesystem::on_fs_init([] {
				init_locals();
			});
		}

		void post_unpack() override {
			command::add("loc_reload", [] {
				init_locals();
			});

			assets::on_asset_load([](s4::XAssetType type, const char* given_name, int allow_create_default, s4::XAssetHeader& res) {
				if (type != s4::XAssetType::ASSET_TYPE_LOCALIZE) {
					return;
				}

				auto it = stored_localize_additions.find(given_name);
				if (it != stored_localize_additions.end()) {
					res.localize_ = it->second;
					res.localize_->value_ = localize_string(given_name);
					return;
				}

				s4::LocalizeEntry* new_entry = new s4::LocalizeEntry();
				new_entry->name_ = given_name;
				new_entry->value_ = localize_string(given_name);
				stored_localize_additions[given_name] = new_entry;
				res.localize_ = new_entry;
			});
		}
	};
}

REGISTER_COMPONENT(localized_strings::component)