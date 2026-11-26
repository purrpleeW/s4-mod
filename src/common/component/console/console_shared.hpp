#pragma once
#include <string>
#include <optional>
#include <deque>
#include <concepts>

namespace console::detail {
	using extended_output_queue = std::deque<std::string>;

	struct console_positioning {
		float left, top, right, bottom;
	};

	enum class suggestion_kind {
		command, dvar, omnvar
	};

	struct suggestion_entry {
		suggestion_kind kind;
		std::string name;
		std::optional<std::string> obfuscated_name;
		std::optional<std::string> description;
		std::uint64_t checksum;
	};

	template <typename C>
	concept generic_vec4 = requires(C c) {
		{ c.r } -> std::same_as<float&>;
		{ c.g } -> std::same_as<float&>;
		{ c.b } -> std::same_as<float&>;
		{ c.a } -> std::same_as<float&>;
		{ c.x } -> std::same_as<float&>;
		{ c.y } -> std::same_as<float&>;
		{ c.z } -> std::same_as<float&>;
		{ c.w } -> std::same_as<float&>;
	};

	enum class console_state {
		hidden,
		input_only,
		extended_output
	};

	// Key codes
	namespace keys {
		constexpr std::uint8_t K_GRAVE = 0x60;
		constexpr std::uint8_t K_TAB = 0x09;
		constexpr std::uint8_t K_ENTER = 0x0D;
		constexpr std::uint8_t K_BACKSPACE = 0x08;
		constexpr std::uint8_t K_CTRL_BACKSPACE = 0x7F;
		constexpr std::uint8_t K_CLEAR = 0x03;
		constexpr std::uint8_t K_CLIPBOARD = 0x16;
		constexpr std::uint8_t K_UPARROW = 0x84;
		constexpr std::uint8_t K_DOWNARROW = 0x85;
		constexpr std::uint8_t K_LEFTARROW = 0x86;
		constexpr std::uint8_t K_RIGHTARROW = 0x87;
		constexpr std::uint8_t K_MWHEELUP = 0xC1;
		constexpr std::uint8_t K_MWHEELDOWN = 0xC0;
		constexpr std::uint8_t K_PGUP = 0x97;
		constexpr std::uint8_t K_PGDN = 0x96;
	}

	// Layout constants
	namespace layout {
		constexpr auto box_border_thickness = 2.f;
		constexpr auto screen_edge_padding = 6.f;
		constexpr auto buffer_size = 0x10000; // 64kb
		constexpr auto max_output_lines = 512;
		constexpr auto max_history_size = 10;
		constexpr auto max_visible_suggestions = 24;
		constexpr auto max_cursor_position = 254;
	}
}