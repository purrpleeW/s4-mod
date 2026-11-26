#include "common.hpp"
#include "branding.hpp"

#include "component/command.hpp"
#include "component/scheduler.hpp"
#include "game/game.hpp"
#include "component/console/console_providers_renderer.hpp"

#include <component/localized_strings.hpp>
#include <identification/utility.hpp>
#include <identification/game.hpp>
#include <loader/component_loader.hpp>
#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/properties.hpp>
#include <utils/string.hpp>

#include <version.hpp>

namespace branding {
	namespace {
		utils::concurrency::container<std::string> watermark_setting{};
		
		std::string parse_watermark_date() {
			static const std::string& timestamp = BUILD_TIMESTAMP;

			static const std::string& yy   = timestamp.substr(0, 2);
			static const std::string& mm   = timestamp.substr(2, 2);
			static const std::string& dd   = timestamp.substr(4, 2);
			static const std::string& hour = timestamp.substr(7, 2);
			static const std::string& min  = timestamp.substr(9, 2);

			return dd + "/" + mm + "/" + yy + " " + hour + ":" + min;
		}

		std::string validate_watermark_setting(const std::string& watermark_setting) {
			if (watermark_setting == "on" || watermark_setting == "off") {
				return watermark_setting;
			}

			return "on";
		}

		void store_watermark_setting(const std::string& watermark_setting) {
			utils::properties::store("watermarkSetting", watermark_setting);
		}

		void activate_watermark_setting(std::string new_watermark_setting) {
			watermark_setting.access([&](std::string& watermark_setting) {
				watermark_setting = std::move(new_watermark_setting);
			});
		}

		void setup_watermark_setting() {
			update_watermark_setting("on");
		}

		void load_watermark_setting() {
			const auto stored_watermark_setting = utils::properties::load("watermarkSetting");

			if (stored_watermark_setting) {
				activate_watermark_setting(validate_watermark_setting(*stored_watermark_setting));
			}
			else {
				setup_watermark_setting();
			}
		}

		void draw_branding() {
			if (!should_show_watermark()) {
				return;
			}

			#if CLIENT_PRERELEASE == TRUE
				static const std::string& watermark_text = std::format(
					"CLIENT-PRERELEASE - {} " BUILD_VERSION_STRING " v{}\n"
					BUILD_HOST_NAME "\\" BUILD_USER_NAME " at {}",
					identification::utility::get_client_name(),
					identification::utility::get_game_version(),
					parse_watermark_date() // format: CLIENT-PRERELEASE - s4-mod (githash) v1.0.0.0 \n DESKTOP-12345\user at 01/01/23 12:00
				);

				auto font = console::get_console_font();
				if (font == nullptr) {
					return;
				}

				static const auto offset_from_corner = 4.0f;
				static s4::vec4_t text_color = { 1.0f, 0.0f, 0.0f, 1.0f };

				console::draw_text(watermark_text, offset_from_corner, offset_from_corner + static_cast<float>(font->pixel_height_), text_color);
			#else
				auto font = console::get_console_font();
				if (font == nullptr) {
					return;
				}

				static const std::string& watermark_text = std::format("{}: " BUILD_VERSION_STRING " [v{}]", identification::utility::get_client_name(),
					identification::utility::get_game_version());

				s4::vec4_t color = { .666f, .666f, .666f, .666f };
				console::draw_text(watermark_text, 4.f, 4.f + static_cast<float>(font->pixel_height_), color);
			#endif
		}
	}

	bool should_show_watermark() {
		const auto setting = watermark_setting.copy();
		return strcmp(utils::string::va("%.*s", static_cast<int>(setting.size()), setting.data()), "on") == 0;
	}

	void update_watermark_setting(const std::string& new_watermark_setting) {
		activate_watermark_setting(validate_watermark_setting(new_watermark_setting));
		store_watermark_setting(validate_watermark_setting(new_watermark_setting));
	}

	struct component final : generic_component {
		void post_load() override {
			load_watermark_setting();
			scheduler::loop<scheduler::pipeline::client>(draw_branding);

			/*
			localized_strings::override("MENU/STATUS", std::format("{}: " BUILD_VERSION_STRING " - v{}",
				identification::utility::get_client_name(), identification::utility::get_game_version()));
			*/
		}

		void post_unpack() override {
			command::add("watermark", [](const command::params_cl& params) {
				if (params.size() != 2) {
					return;
				}

				update_watermark_setting(std::string(params[1]));
			});
		}
	};
}

REGISTER_COMPONENT(branding::component)
