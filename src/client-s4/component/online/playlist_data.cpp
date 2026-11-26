#include "playlist_data.hpp"

#include <nlohmann/json.hpp>
#include <zlib.h>

#include <limits>
#include <stdexcept>

namespace online::playlists {
	namespace {
		std::expected<std::string, std::string> decompress_playlist(std::string_view input) {
			std::string output(playlist_file_size_limit, '\0');
			uLongf output_size = static_cast<uLongf>(output.size());
			uLong input_size = static_cast<uLong>(input.size());
			const auto result = uncompress2(reinterpret_cast<Bytef*>(output.data()), &output_size,
				reinterpret_cast<const Bytef*>(input.data()), &input_size);

			if (result != Z_OK || input_size != input.size()) {
				return std::unexpected("Invalid zlib compressed playlist");
			}

			output.resize(output_size);
			return output;
		}

		bool is_positive_version(const nlohmann::json& value) {
			return value.is_number_integer() && value > 0 && value <= std::numeric_limits<std::int32_t>::max();
		}
	}

	std::expected<playlist_document, std::string> parse_playlist_document(std::string_view input) {
		if (input.empty() || input.size() > playlist_file_size_limit) {
			return std::unexpected("Playlist file is empty or exceeds 16 MB");
		}

		std::string decompressed;
		const auto zlib_header = input.size() >= 2
			? (static_cast<unsigned char>(input[0]) << 8) | static_cast<unsigned char>(input[1]) : 0;
		if ((zlib_header >> 8 & 15) == Z_DEFLATED && zlib_header % 31 == 0) {
			auto result = decompress_playlist(input);
			if (!result) {
				return std::unexpected(result.error());
			}

			decompressed = std::move(*result);
			input = decompressed;
		}

		try {
			auto document = nlohmann::json::parse(input.begin(), input.end(), [](int depth, auto, auto&) {
				if (depth > 64) {
					throw std::runtime_error("Playlist JSON nesting exceeds 64");
				}
				return true;
			});

			if (!document.is_object() || !document.contains("system") || !document["system"].is_object()
				|| !document["system"].contains("version") || !is_positive_version(document["system"]["version"])) {
				return std::unexpected("Playlist requires a positive int system.version");
			}

			if (!document.contains("global_dvars") || !document["global_dvars"].is_object()
				|| !document["global_dvars"].contains("crc32")) {
				return std::unexpected("Playlist needs an unsigned 32-bit global_dvars.crc32");
			}

			const auto& crc32 = document["global_dvars"]["crc32"];
			if (!crc32.is_number_integer() || crc32 < 0 || crc32 > std::numeric_limits<std::uint32_t>::max()) {
				return std::unexpected("Playlist needs an unsigned 32-bit global_dvars.crc32");
			}

			for (const auto* key : { "gametypes", "playlists", "categories" }) {
				if (!document.contains(key) || !document[key].is_array() || document[key].empty()) {
					return std::unexpected(std::string("Playlist needs a non-empty array: ") + key);
				}

				for (const auto& entry : document[key]) {
					if (!entry.is_object()) {
						return std::unexpected(std::string("Playlist array must contain objects: ") + key);
					}
				}
			}

			if (document["gametypes"].size() > 96 || document["playlists"].size() > 1024 || document["categories"].size() > 27) {
				return std::unexpected("Playlist exceeds the supported table limits");
			}

			auto content = document.dump();
			if (content.size() >= playlist_buffer_size) {
				return std::unexpected("Minified playlist must fit in 1 MB");
			}

			return playlist_document{ std::move(content), document["system"]["version"].get<std::uint32_t>(), crc32.get<std::uint32_t>(),
				document["gametypes"].size(), document["playlists"].size(), document["categories"].size() };
		}
		catch (const nlohmann::json::exception& error) {
			return std::unexpected(std::string("Invalid playlist JSON: ") + error.what());
		}

		catch (const std::runtime_error& error) {
			return std::unexpected(error.what());
		}
	}
}