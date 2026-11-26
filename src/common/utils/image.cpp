#include "image.hpp"
#include <stdexcept>

#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include "safe_release.hpp"

namespace utils::image {
	image load_image(const std::string& data, bool animated) {
		stbi_uc* buffer{};
		int* delays{};
		const auto _ = safe_release([&buffer, &delays, &animated] {
			if (buffer) {
				stbi_image_free(buffer);
			}
			if (animated && delays) {
				free(delays);
			}
		});

		constexpr int channels = 4;
		int x, y, channels_in_file;
		int frame_count = 1;
		if (animated) {
			buffer = stbi_load_gif_from_memory(PTR_AS(const std::uint8_t*, data.data()), VAL_AS(int, data.size()), &delays, &x, &y, &frame_count,
				&channels_in_file, channels);
		}
		else {
			buffer = stbi_load_from_memory(PTR_AS(const std::uint8_t*, data.data()), VAL_AS(int, data.size()), &x, &y, &channels_in_file, channels);
		}
		if (!buffer) {
			throw std::runtime_error("Failed to load image");
		}

		image res{};
		res.width = VAL_AS(std::size_t, x);
		res.height = VAL_AS(std::size_t, y);
		res.data.assign(PTR_AS(const char*, buffer), res.width * res.height * channels * frame_count);
		res.frames = frame_count;
		if (animated && delays) {
			res.frame_delays.reserve(VAL_AS(std::size_t, frame_count));
			for (auto x = 0; x < frame_count; x++) {
				res.frame_delays.push_back(delays[x]);
			}
		}

		return res;
	}

	object create_bitmap(const image& img) {
		auto copy = img.data;

		for (std::size_t i = 0; i < (img.width * img.height); ++i) {
			auto& r = copy[i * 4 + 0];
			auto& b = copy[i * 4 + 2];
			std::swap(r, b);
		}

		return CreateBitmap(VAL_AS(int, img.width), VAL_AS(int, img.height), 4, 8, copy.data());
	}
}
