#pragma once

#include "component/assets/camo/camo_animation.hpp"

#include <algorithm>
#include <charconv>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace assets::camo::settings {
	using json = nlohmann::json;

	inline std::optional<std::string> display_name(const json& document, std::vector<std::string>& errors) {
		if (!document.is_object() || !document.contains("name")) {
			return {};
		}

		const auto& value = document["name"];
		if (value.is_string()) {
			const auto& text = value.get_ref<const std::string&>();
			const bool printable = std::all_of(text.begin(), text.end(), [](unsigned char character) {
				return character >= 32 && character != 127;
			});

			if (!text.empty() && text.size() <= 256 && printable && text.find_first_not_of(' ') != std::string::npos) {
				return text;
			}
		}

		errors.emplace_back("name must not be empty, longer than 256 characters, or contain non-printable characters");
		return {};
	}

	inline std::optional<std::uint32_t> uint32(const json& value) {
		if (value.is_number_unsigned()) {
			const auto number = value.get<std::uint64_t>();
			if (number <= std::numeric_limits<std::uint32_t>::max()) {
				return static_cast<std::uint32_t>(number);
			}
		}
		else if (value.is_number_integer()) {
			const auto number = value.get<std::int64_t>();
			if (number >= 0 && number <= std::numeric_limits<std::uint32_t>::max()) {
				return static_cast<std::uint32_t>(number);
			}
		}

		return {};
	}

	inline std::optional<std::uint32_t> color(const json& value) {
		if (!value.is_string()) {
			return uint32(value);
		}

		const auto& text = value.get_ref<const std::string&>();
		if (text.empty()) {
			return {};
		}

		if (text.find(':') != std::string::npos) {
			std::uint32_t result{};
			std::size_t start{};
			for (unsigned index = 0; index < 4; ++index) {
				const auto end = text.find(':', start);
				if ((index < 3 && end == std::string::npos) || (index == 3 && end != std::string::npos)) {
					return {};
				}

				const auto last = end == std::string::npos ? text.size() : end;
				unsigned byte{};
				const auto parsed = std::from_chars(text.data() + start, text.data() + last, byte);
				if (parsed.ec != std::errc{} || parsed.ptr != text.data() + last || byte > 255) {
					return {};
				}

				result = (result << 8) | byte;
				start = last + 1;
			}

			return result;
		}

		const bool hex = text.starts_with("0x") || text.starts_with("0X");
		std::uint32_t result{};
		const auto parsed = std::from_chars(text.data() + (hex ? 2 : 0), text.data() + text.size(), result, hex ? 16 : 10);
		if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
			return {};
		}

		return result;
	}

	inline std::optional<float> number(const json& value, double maximum = 10000) {
		if (!value.is_number()) {
			return {};
		}

		const auto result = value.get<double>();
		if (!std::isfinite(result) || std::abs(result) > maximum) {
			return {};
		}

		return static_cast<float>(result);
	}

	template <std::size_t N>
	std::optional<std::array<float, N>> numbers(const json& value, double maximum = 10000) {
		if (!value.is_array() || value.size() != N) {
			return {};
		}

		std::array<float, N> result{};
		for (std::size_t index = 0; index < N; ++index) {
			const auto item = number(value[index], maximum);
			if (!item) {
				return {};
			}

			result[index] = *item;
		}

		return result;
	}

	namespace detail {
		inline bool read_packed_properties(
			const json& layer, const std::string& key, animation::properties& base, std::size_t index, std::vector<std::string>& errors) {
			const std::array fields = { std::pair{ "color_tint", &base.tints[index] },
				std::pair{ "albedo_map_scale_bias", &base.albedo[index] }, std::pair{ "normal_map_scale_bias", &base.normal[index] } };
			bool changed = false;

			for (const auto& [field, target] : fields) {
				if (!layer.contains(field)) {
					continue;
				}

				const auto value = std::string_view(field) == "color_tint" ? color(layer[field]) : uint32(layer[field]);
				if (!value) {
					errors.emplace_back(key + "." + field + " must be a valid color");
					continue;
				}

				*target = *value;
				changed = true;
			}

			return changed;
		}

		inline bool read_uv_properties(
			const json& layer, const std::string& key, animation::vector4& uv, std::vector<std::string>& errors) {
			bool changed = false;

			for (const auto* field : { "uv_scale", "uv_offset" }) {
				if (!layer.contains(field)) {
					continue;
				}

				const auto value = numbers<2>(layer[field]);
				if (!value) {
					errors.emplace_back(key + "." + field + " must contain two finite numbers");
					continue;
				}

				if (std::string_view(field) == "uv_scale") {
					uv.x = (*value)[0];
					uv.y = (*value)[1];
				}
				else {
					uv.z = (*value)[0];
					uv.w = (*value)[1];
				}

				changed = true;
			}

			return changed;
		}

		inline animation::layer read_layer_animation(const json& fields, const std::string& key, std::vector<std::string>& errors) {
			animation::layer layer{};

			if (fields.contains("scroll")) {
				if (const auto value = numbers<2>(fields["scroll"], 1000)) {
					layer.scroll = *value;
				}
				else {
					errors.emplace_back(key + ".scroll must contain two finite speeds");
				}
			}

			if (fields.contains("rotate")) {
				if (const auto value = number(fields["rotate"], 1000)) {
					layer.rotate_speed = *value;
				}
				else {
					errors.emplace_back(key + ".rotate must be a finite speed");
				}
			}

			if (fields.contains("pulse")) {
				const auto value = numbers<3>(fields["pulse"], 1000);
				if (value && (*value)[1] >= 0 && (*value)[2] >= (*value)[1] && (*value)[2] <= 100) {
					layer.pulse_speed = (*value)[0];
					layer.pulse_min = (*value)[1];
					layer.pulse_max = (*value)[2];
				}
				else {
					errors.emplace_back(key + ".pulse must be [speed, min, max] with 0 <- min <- max <- 100");
				}
			}

			if (fields.contains("color_cycle")) {
				const auto& cycle = fields["color_cycle"];
				const auto value = cycle.is_number() ? numbers<2>(json::array({ cycle, 1.f }), 1000) : numbers<2>(cycle, 1000);
				if (value && (*value)[1] >= 0 && (*value)[1] <= 1) {
					layer.color_cycle_speed = (*value)[0];
					layer.color_cycle_brightness = (*value)[1];
				}
				else {
					errors.emplace_back(key + ".color_cycle must be speed or [speed, brightness 0..1]");
				}
			}

			return layer;
		}
	}

	inline bool properties(const json& document, animation::properties& base, std::vector<std::string>& errors) {
		if (!document.is_object()) {
			errors.emplace_back("properties must be an object");
			return false;
		}

		bool changed = false;
		if (document.contains("icon")) {
			errors.emplace_back("icon is not ported to S4-Mod yet, try later :3");
		}

		if (document.contains("blend_map_channels")) {
			const auto value = uint32(document["blend_map_channels"]);
			if (value && *value <= std::numeric_limits<std::uint8_t>::max()) {
				base.blend_map_channels = static_cast<std::uint8_t>(*value);
				changed = true;
			}
			else {
				errors.emplace_back("blend_map_channels must be an integer from 0 to 255");
			}
		}

		for (std::size_t index = 0; index < 3; ++index) {
			const auto key = "level" + std::to_string(index);
			if (!document.contains(key)) {
				continue;
			}

			const auto& layer = document[key];
			if (!layer.is_object()) {
				errors.emplace_back(key + " must be an object");
				continue;
			}

			changed |= detail::read_packed_properties(layer, key, base, index, errors);
			changed |= detail::read_uv_properties(layer, key, base.uv[index], errors);
		}

		return changed;
	}

	inline animation::configuration animation_config(const json& document, std::vector<std::string>& errors) {
		animation::configuration config{};
		if (!document.is_object()) {
			errors.emplace_back("animation must be an object");
			return config;
		}

		for (std::size_t index = 0; index < 3; ++index) {
			const auto key = "level" + std::to_string(index);
			if (!document.contains(key)) {
				continue;
			}

			const auto& fields = document[key];
			if (!fields.is_object()) {
				errors.emplace_back(key + " must be an object");
				continue;
			}

			config[index] = detail::read_layer_animation(fields, key, errors);
		}

		return config;
	}
}
