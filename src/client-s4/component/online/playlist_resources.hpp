#pragma once

#include "resource.h"

#include <cstdint>
#include <string_view>

namespace online::playlists {
	struct playlist_resource {
		std::string_view game_version;
		int resource_id;
		std::uint32_t revision;
		std::string_view filename;
	};

	// only did these, had to estimate which playlists went to which version (1.24.1-ES is entirely a guess)
	inline constexpr playlist_resource playlist_resources[] = {
		{ "1.07.1", DW_PLAYLIST_REV29, 29, "unified_playlist.aggr29.txt" },
		{ "1.10.0", DW_PLAYLIST_REV47, 47, "unified_playlist.aggr47.txt" },
		{ "1.14.0", DW_PLAYLIST_REV70, 70, "unified_playlist.aggr70.txt" },
		{ "1.16.0", DW_PLAYLIST_REV86, 86, "unified_playlist.aggr86.txt" },
		{ "1.24.1-ES", DW_PLAYLIST_REV86, 86, "unified_playlist.aggr178.txt" },
		{ "1.27.0", DW_PLAYLIST_REV182, 182, "unified_playlist.aggr182.txt" }
	};

	constexpr const playlist_resource* find_playlist_resource(std::string_view game_version) {
		for (const auto& resource : playlist_resources) {
			if (resource.game_version == game_version) {
				return &resource;
			}
		}

		return nullptr;
	}
}