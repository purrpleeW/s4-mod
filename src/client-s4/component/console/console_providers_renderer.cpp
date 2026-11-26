#include "game/function_types.hpp"

#include <component/console/console_providers.hpp>
#include <loader/component_loader.hpp>

namespace console {
	namespace {
		functions::AddBaseDrawTextCmdT* AddBaseDrawTextCmd{};
		functions::R_TextWidthT* R_TextWidth{};
		functions::R_RegisterFontT* R_RegisterFont{};
		functions::UI_FillRectPhysicalT* UI_FillRectPhysical{};

		s4::vec2_t* scrPlaceFull_realViewportSize{};
		s4::CachedAssets_t* sharedUiInfo_assets{};

		void R_AddCmdDrawText(const char* text, int max_chars, s4::GfxFont* font, int font_height, float x, float y, float x_scale, float y_scale,
			float rotation, const s4::vec4_t* color)
		{
			AddBaseDrawTextCmd(text, max_chars, font, nullptr, font_height, x, y, x_scale, y_scale, '\0',
				rotation, color, -1, '\0', nullptr, false, 0, 0, nullptr, false
			);
		}
	}

	namespace detail {
		using renderer = rendering_provider<s4::GfxFont, s4::vec4_t>;

		template <>
		s4::GfxFont* renderer::get_console_font() {
			static s4::GfxFont* con_font = R_RegisterFont("fonts/fira_mono_regular.ttf", 18);
			if (con_font != nullptr) {
				return con_font;
			}
			return sharedUiInfo_assets->text_font_;
		}

		template <>
		int renderer::get_font_height_in_px() {
			return renderer::get_console_font()->pixel_height_;
		}

		template <>
		float renderer::get_text_width(const std::string &str) {
			return R_TextWidth(str.c_str(), str.length(), renderer::get_console_font());
		}

		template <>
		std::pair<float, float> renderer::get_viewport_dimensions() {
			return std::make_pair(scrPlaceFull_realViewportSize->x, scrPlaceFull_realViewportSize->y);
		}

		template <>
		void renderer::draw_text(const std::string &str, float x, float y, s4::vec4_t color_vec) {
			return R_AddCmdDrawText(str.c_str(), str.length(),
				renderer::get_console_font(), renderer::get_font_height_in_px(),
				x, y, 1.f, 1.f, 0.f, color_vec.get_ptr()
			);
		}

		template <>
		void renderer::draw_rect(s4::vec4_t dimensions, s4::vec4_t color_vec) {
			return UI_FillRectPhysical(dimensions.x, dimensions.y, dimensions.z, dimensions.w, color_vec.get_ptr());
		}

		template <>
		void renderer::draw_text_with_cursor(const std::string &str, float x, float y, int cursor_pos, s4::vec4_t color_vec) {
			AddBaseDrawTextCmd(str.c_str(), str.size(),
				renderer::get_console_font(), nullptr,
				renderer::get_font_height_in_px(), x, y, 1.f, 1.f,
				'\0', 0.f, color_vec.get_ptr(), cursor_pos, '|',
				nullptr, false, 0, 0, nullptr, false
			);
		}
	}

	void draw_text(const std::string& str, float x, float y, s4::vec4_t color) {
		return detail::renderer::draw_text(str, x, y, color);
	}

	void draw_rect(s4::vec4_t dimensions, s4::vec4_t color) {
		return detail::renderer::draw_rect(dimensions, color);
	}

	void draw_text_with_cursor(const std::string& str, float x, float y, int cursor_pos, s4::vec4_t color) {
		return detail::renderer::draw_text_with_cursor(str, x, y, cursor_pos, color);
	}

	s4::GfxFont* get_console_font() {
		return detail::renderer::get_console_font();
	}

	int get_font_height_in_px() {
		return detail::renderer::get_font_height_in_px();
	}

	float get_text_width(const std::string& str) {
		return detail::renderer::get_text_width(str);
	}

	std::pair<float, float> get_viewport_dimensions() {
		return detail::renderer::get_viewport_dimensions();
	}

	namespace providers::renderer {
		struct component final : generic_component {
			component_priority priority() const override {
				return component_priority::pointers;
			}

			void find_signatures(memory::signature_store& batch) override {
				batch.add(SETUP_POINTER(AddBaseDrawTextCmd), "E8 ? ? ? ? 48 8B D0 48 85 C0 0F 84 ? ? ? ? 48 8B 8C 24",
					SETUP_MOD(add(1).rip()));

				batch.add(SETUP_POINTER(R_TextWidth), "48 83 EC ? C5 FA 10 05 ? ? ? ? 45 8B 48");

				if (identification::game::is_config(identification::game::Beta)) {
					batch.add(SETUP_POINTER(R_RegisterFont), "E8 ? ? ? ? BA ? ? ? ? 48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? "
						"BA ? ? ? ? 48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 8B 05", SETUP_MOD(add(1).rip()));
				}
				else {
					batch.add(SETUP_POINTER(R_RegisterFont), "E8 ? ? ? ? 48 6B 17", SETUP_MOD(add(1).rip()));
				}

				batch.add(SETUP_POINTER(UI_FillRectPhysical), "48 83 EC ? 48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 89 44 24 ? 48 8B 84 "
					"24 ? ? ? ? C5 D8 57 E4");

				batch.add(SETUP_POINTER(scrPlaceFull_realViewportSize), "48 8D 0D ? ? ? ? 8B 01 89 02", SETUP_MOD(add(3).rip()));

				if (identification::game::is_config(identification::game::Beta)) {
					batch.add(SETUP_POINTER(sharedUiInfo_assets), "48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? B2 ? 48 89 05 ? ? ? ? "
						"48 8D 0D ? ? ? ? E8 ? ? ? ? 33 D2", SETUP_MOD(add(3).rip()));
				}
				else {
					batch.add(SETUP_POINTER(sharedUiInfo_assets), "48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? B2 ? 48 89 05 ? ? ? ? "
						"48 8D 0D ? ? ? ? E8 ? ? ? ? 80 3D", SETUP_MOD(add(3).rip()));
				}
			}
		};
	}
}

REGISTER_COMPONENT(console::providers::renderer::component)