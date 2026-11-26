#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace online::playlists {
	inline constexpr std::size_t playlist_buffer_size = 1024 * 1024;
	inline constexpr std::size_t playlist_file_size_limit = 16 * 1024 * 1024;

	struct playlist_document {
		std::string content;
		std::uint32_t version;
		std::uint32_t global_dvars_crc32;
		std::size_t gametype_count;
		std::size_t playlist_count;
		std::size_t category_count;
	};

	std::expected<playlist_document, std::string> parse_playlist_document(std::string_view input);
}