#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace assets::camo::animation {
	struct vector4 {
		float x{};
		float y{};
		float z{};
		float w{};
	};

	struct properties {
		std::uint8_t blend_map_channels{};
		std::array<std::uint32_t, 3> tints{};
		std::array<std::uint32_t, 3> albedo{};
		std::array<std::uint32_t, 3> normal{};
		std::array<vector4, 3> uv{};
		std::array<vector4, 3> rotation{};
	};

	struct layer {
		std::array<float, 2> scroll{};
		float rotate_speed{};
		float pulse_speed{};
		float pulse_min{ 1.f };
		float pulse_max{ 1.f };
		float color_cycle_speed{};
		float color_cycle_brightness{ 1.f };

		bool active() const {
			return scroll[0] != 0 || scroll[1] != 0 || rotate_speed != 0 || pulse_speed != 0 || color_cycle_speed != 0;
		}
	};

	using configuration = std::array<layer, 3>;

	inline bool active(const configuration& config) {
		return std::any_of(config.begin(), config.end(), [](const layer& value) {
			return value.active();
		});
	}

	inline float wrap(double value) {
		return static_cast<float>(value - std::floor(value));
	}

	inline std::uint32_t cycle_color(double phase, float brightness, std::uint32_t alpha) {
		const auto hue = (phase - std::floor(phase)) * 6;
		const auto b = std::clamp(brightness, 0.f, 1.f);
		const auto x = b * static_cast<float>(1 - std::abs(std::fmod(hue, 2) - 1));
		float r{}, g{}, blue{};
		if (hue < 1) {
			r = b;
			g = x;
		}
		else if (hue < 2) {
			r = x;
			g = b;
		}
		else if (hue < 3) {
			g = b;
			blue = x;
		}
		else if (hue < 4) {
			g = x;
			blue = b;
		}
		else if (hue < 5) {
			r = x;
			blue = b;
		}
		else {
			r = b;
			blue = x;
		}

		return (alpha & 0xFF000000u) | static_cast<std::uint32_t>(r * 255.f) | (static_cast<std::uint32_t>(g * 255.f) << 8) |
			(static_cast<std::uint32_t>(blue * 255.f) << 16);
	}

	inline properties evaluate(const properties& base, const configuration& config, double elapsed) {
		auto result = base;
		if (!std::isfinite(elapsed)) {
			return result;
		}

		constexpr double tau = 6.2831853071795864769;
		for (std::size_t index = 0; index < config.size(); ++index) {
			const auto& layer = config[index];
			auto& uv = result.uv[index];
			if (layer.scroll[0] != 0) {
				uv.z = wrap(base.uv[index].z + elapsed * layer.scroll[0]);
			}

			if (layer.scroll[1] != 0) {
				uv.w = wrap(base.uv[index].w + elapsed * layer.scroll[1]);
			}

			if (layer.pulse_speed != 0) {
				const auto phase = (std::sin(std::fmod(elapsed * layer.pulse_speed, tau)) + 1) * 0.5;
				const auto scale = static_cast<float>(layer.pulse_min + phase * (layer.pulse_max - layer.pulse_min));
				uv.x *= scale;
				uv.y *= scale;
			}

			if (layer.rotate_speed != 0) {
				const auto angle = std::fmod(elapsed * layer.rotate_speed, tau);
				const auto cosine = static_cast<float>(std::cos(angle));
				const auto sine = static_cast<float>(std::sin(angle));
				const auto& matrix = base.rotation[index];
				result.rotation[index] = { matrix.x * cosine + matrix.y * sine, -matrix.x * sine + matrix.y * cosine,
					matrix.z * cosine + matrix.w * sine, -matrix.z * sine + matrix.w * cosine };
			}

			if (layer.color_cycle_speed != 0) {
				result.tints[index] = cycle_color(elapsed * layer.color_cycle_speed, layer.color_cycle_brightness, base.tints[index]);
			}
		}

		return result;
	}
}
