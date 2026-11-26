#pragma once

#include <cstddef>
#include <string_view>

namespace assets::image::validation {
	inline constexpr std::size_t max_dimension = 2048;
	inline constexpr std::size_t max_frames = 128;
	inline constexpr std::size_t max_file_size = 32 * 1024 * 1024;
	inline constexpr std::size_t max_decoded_size = 128 * 1024 * 1024;

	inline std::size_t rgba_frame_size(int width, int height) {
		if (width <= 0 || height <= 0 || width > static_cast<int>(max_dimension) || height > static_cast<int>(max_dimension)) {
			return 0;
		}

		return static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4;
	}

	inline bool is_gif(std::string_view bytes) {
		return bytes.starts_with("GIF87a") || bytes.starts_with("GIF89a");
	}

	// 3/4 written by claude here, literally had nightmares throwing this together
	inline bool validate_gif(std::string_view bytes, std::size_t frame_size) { 
		if (!frame_size || frame_size > max_decoded_size || bytes.size() < 13 || !is_gif(bytes)) {
			return false;
		}

		auto byte = [&](std::size_t pos) {
			return static_cast<unsigned char>(bytes[pos]);
		};

		auto word = [&](std::size_t pos) {
			return byte(pos) | (byte(pos + 1) << 8);
		};

		const auto width = word(6), height = word(8);
		if (rgba_frame_size(width, height) != frame_size) {
			return false;
		}

		std::size_t pos = 13;
		if (byte(10) & 0x80) {
			const auto table_size = 3u << ((byte(10) & 7) + 1);
			if (table_size > bytes.size() - pos) {
				return false;
			}

			pos += table_size;
		}

		auto skip_blocks = [&]() {
			while (pos < bytes.size()) {
				const auto size = byte(pos++);
				if (!size) {
					return true;
				}

				if (size > bytes.size() - pos) {
					return false;
				}

				pos += size;
			}

			return false;
		};

		std::size_t frames = 0;
		while (pos < bytes.size()) {
			switch (byte(pos++)) {
			case 0x3B:
				return frames != 0;
			case 0x21:
				if (pos >= bytes.size()) {
					return false;
				}

				++pos;
				if (!skip_blocks()) {
					return false;
				}

				break;
			case 0x2C: {
				if (bytes.size() - pos < 9 || ++frames > max_frames || frames > max_decoded_size / frame_size) {
					return false;
				}

				const auto left = word(pos), top = word(pos + 2);
				const auto image_width = word(pos + 4), image_height = word(pos + 6);
				if (!image_width || !image_height || left + image_width > width || top + image_height > height) {
					return false;
				}

				const auto packed = byte(pos + 8);
				pos += 9;
				if (packed & 0x80) {
					const auto table_size = 3u << ((packed & 7) + 1);
					if (table_size > bytes.size() - pos) {
						return false;
					}

					pos += table_size;
				}

				if (pos >= bytes.size() || byte(pos) < 2 || byte(pos) > 8) {
					return false;
				}

				++pos;
				if (!skip_blocks()) {
					return false;
				}

				break;
			}

			default:
				return false;
			}
		}

		return false;
	}
}