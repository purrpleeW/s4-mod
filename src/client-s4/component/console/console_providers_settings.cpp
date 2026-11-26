#include "game/function_types.hpp"
#include "game/dvars.hpp"

#include <component/console/console_providers.hpp>
#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

namespace console {
	namespace detail {
		using settings = settings_provider<s4::vec4_t>;

		template <>
		s4::vec4_t settings::get_input_box_color() {
			return dvars::get_vector(dvars::con_inputBoxColor);
		}

		template <>
		s4::vec4_t settings::get_input_title_color() {
			return dvars::get_vector(dvars::con_inputTitleColor);
		}

		template <>
		s4::vec4_t settings::get_input_text_color() {
			return dvars::get_vector(dvars::con_inputTextColor);
		}

		template <>
		s4::vec4_t settings::get_hint_box_color() {
			return dvars::get_vector(dvars::con_inputHintBoxColor);
		}

		template <>
		s4::vec4_t settings::get_hint_match_description_color() {
			return dvars::get_vector(dvars::con_inputHintDescColor);
		}

		template <>
		s4::vec4_t settings::get_hint_match_description_unavailable_color() {
			return dvars::get_vector(dvars::con_inputHintDescUnavailableColor);
		}

		template <>
		s4::vec4_t settings::get_hint_match_cmd_color() {
			return dvars::get_vector(dvars::con_inputCmdMatchColor);
		}

		template <>
		s4::vec4_t settings::get_hint_match_dvar_color() {
			return dvars::get_vector(dvars::con_inputDvarMatchColor);
		}

		template <>
		s4::vec4_t settings::get_dvar_display_value_color() {
			return dvars::get_vector(dvars::con_inputDvarValueColor);
		}

		template <>
		s4::vec4_t settings::get_dvar_default_value_color() {
			return dvars::get_vector(dvars::con_inputDvarInactiveValueColor);
		}

		template <>
		s4::vec4_t settings::get_dvar_domain_color() {
			return dvars::get_vector(dvars::con_inputCmdMatchColor); //@ todo(luxploit): probably change this to another color
		}

		template <>
		s4::vec4_t settings::get_extended_output_box_color() {
			return dvars::get_vector(dvars::con_outputWindowColor);
		}

		template <>
		s4::vec4_t settings::get_extended_output_scrollbar_background_color() {
			return dvars::get_vector(dvars::con_outputBarColor);
		}

		template <>
		s4::vec4_t settings::get_extended_output_scrollbar_slider_color() {
			return dvars::get_vector(dvars::con_outputSliderColor);
		}

		template <>
		s4::vec4_t settings::get_extended_output_text_color() {
			return dvars::get_vector(dvars::con_outputTextColor);
		}
	}

	namespace providers::settings {
		struct component final : generic_component {
			void post_load() override {
				scheduler::once<scheduler::pipeline::dvars>([] {
					const auto console_flag = 0 /* game::DVAR_FLAG_NONE */;

				dvars::con_inputBoxColor = dvars::register_vec4("con_inputBoxColor", 0.2f, 0.2f, 0.2f, 0.9f,
					0.0f, 1.0f, console_flag, "color of console input box");
				dvars::con_inputCmdMatchColor = dvars::register_vec4("con_inputCmdMatchColor", 0.80f, 0.80f, 1.0f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console matched command");
				dvars::con_inputDvarInactiveValueColor = dvars::register_vec4("con_inputDvarInactiveValueColor", 0.8f, 0.8f, 0.8f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console inactive dvar value");
				dvars::con_inputDvarMatchColor = dvars::register_vec4("con_inputDvarMatchColor", 1.0f, 1.0f, 0.8f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console matched dvar");
				dvars::con_inputDvarValueColor = dvars::register_vec4("con_inputDvarValueColor", 1.0f, 1.0f, 0.8f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console matched dvar value");
				dvars::con_inputHintBoxColor = dvars::register_vec4("con_inputHintBoxColor", 0.3f, 0.3f, 0.3f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console input hint box");
				dvars::con_inputHintDescColor = dvars::register_vec4("con_inputHintDescColor", 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console input hint text when no description is available");
				dvars::con_inputHintDescUnavailableColor = dvars::register_vec4("con_inputHintDescUnavailableColor", 0.66f, 0.66f, 0.66f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console input hint text when no description is available");
				dvars::con_inputTextColor = dvars::register_vec4("con_inputTextColor", 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console input title");
				dvars::con_inputTitleColor = dvars::register_vec4("con_inputTitleColor", 1.0f, 0.89f, 0.43f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console input title");
				dvars::con_outputBarColor = dvars::register_vec4("con_outputBarColor", 0.5f, 0.5f, 0.5f, 0.6f,
					0.0f, 1.0f, console_flag, "color of console output bar");
				dvars::con_outputSliderColor = dvars::register_vec4("con_outputSliderColor", 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console output slider");
				dvars::con_outputTextColor = dvars::register_vec4("con_outputTextColor", 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 1.0f, console_flag, "color of console output text");
				dvars::con_outputWindowColor = dvars::register_vec4("con_outputWindowColor", 0.25f, 0.25f, 0.25f, 0.85f,
					0.0f, 1.0f, console_flag, "color of console output window");
				});
			}
		};
	}
}

REGISTER_COMPONENT(console::providers::settings::component)