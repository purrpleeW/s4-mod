#include "common.hpp"
#include "component/online/playlists.hpp"
#include "component/online/playlist_data.hpp"
#include "component/online/playlist_resources.hpp"

#include "component/assets.hpp"
#include "component/auth.hpp"
#include "component/command.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/nt.hpp>

namespace online::playlists {
	namespace {
		functions::Playlist_ParsePlaylistsT* Playlist_ParsePlaylists{};
		functions::UI_ValidatePlaylistNumT* UI_ValidatePlaylistNum{};
		void(*unk_ui_update_playlist_filters)(){};
		bool* g_playlistsFetched{};
		bool* s_playlistOutOfDate{};
		std::uint32_t* s_globalDvarsCRC32{};
		int* s_lastGametypeIndex{};

		struct playlist_source {
			playlist_document document;
			std::string name;
		};

		std::optional<playlist_source> source;
		std::string checked_filename;
		bool explicit_source = false;
		bool requested = false;
		bool parse_failed = false;
		std::atomic<bool> stats_initialized = false;

		bool native_ready() {
			return Playlist_ParsePlaylists && UI_ValidatePlaylistNum && unk_ui_update_playlist_filters
				&& g_playlistsFetched && s_playlistOutOfDate && s_globalDvarsCRC32 && s_lastGametypeIndex
				&& game::Com_FrontEnd_IsInFrontEnd;
		}

		std::string get_playlist_filename() {
			if (!game::Dvar_FindMalleableVar) {
				return "unified_playlist.aggr";
			}

			const auto* filename = dvars::get_string(dvars::get_dvar_by_name("playlistAggrFilename"));
			return filename && *filename ? filename : "unified_playlist.aggr";
		}

		std::expected<playlist_source, std::string> read_playlist(const std::string& filename) {
			const auto relative_path = std::filesystem::path(filename);
			if (filename.empty() || relative_path.has_parent_path() || filename == "." || filename == "..") {
				return std::unexpected("Use a filename inside asset_replacer/playlist, not an absolute or parent path");
			}

			try {
				const auto base = assets::get_content_path("playlist", filename);
				// obviously try an editable json over a txt or aggr
				for (const auto& path : { std::filesystem::path(base.string() + ".json"),
					std::filesystem::path(base.string() + ".txt"), base }) {
					if (!std::filesystem::exists(path)) {
						continue;
					}

					const auto size = std::filesystem::file_size(path);
					if (size == 0 || size > playlist_file_size_limit) {
						return std::unexpected(std::format("{} is empty or exceeds 16 MB", path.string()));
					}

					std::ifstream file(path, std::ios::binary);
					std::string content(static_cast<std::size_t>(size), '\0');
					if (!file.read(content.data(), static_cast<std::streamsize>(content.size()))) {
						return std::unexpected(std::format("Failed to read {}", path.string()));
					}

					auto document = parse_playlist_document(content);
					if (!document) {
						return std::unexpected(std::format("{}: {}", path.string(), document.error()));
					}

					return playlist_source{ std::move(*document), path.string() };
				}

				if (filename != "unified_playlist.aggr") {
					return std::unexpected(std::format("No playlist named '{}'", filename));
				}

				const auto game_version = identification::game::get_version(false);
				const auto* resource = find_playlist_resource(game_version);
				if (!resource) {
					return std::unexpected(std::format("No playlist selected for {} yet! We're proceeding without it", game_version));
				}

				auto document = parse_playlist_document(utils::nt::load_resource(resource->resource_id));
				if (!document) {
					return std::unexpected(document.error());
				}

				if (document->version != resource->revision) {
					return std::unexpected(std::format("Embedded {} has version {}, expected {}?", resource->filename, document->version, resource->revision));
				}

				return playlist_source{ std::move(*document), std::format("embedded {}", resource->filename) };
			}
			catch (const std::exception& error) {
				return std::unexpected(error.what());
			}
		}

		bool prepare_playlist(const std::string& filename) {
			auto result = read_playlist(filename);
			if (!result) {
				LOG("Component/Online/Playlists", WARN, "{}", result.error());
				return false;
			}

			source = std::move(*result);
			requested = false;
			parse_failed = false;
			*g_playlistsFetched = false;
			return true;
		}

		void load_playlist() {
			if (!native_ready() || !source || !stats_initialized || !game::Com_FrontEnd_IsInFrontEnd() || playlist_loaded()) {
				requested = false;
				return;
			}

			const auto& document = source->document;
			Playlist_ParsePlaylists(document.content.c_str());
			if (*s_globalDvarsCRC32 != document.global_dvars_crc32 || *s_lastGametypeIndex != static_cast<int>(document.gametype_count) - 1) {
				parse_failed = true;
				requested = false;
				LOG("Component/Online/Playlists", ERROR, "Parser rejected {} (global dvars CRC32 {} / {}, last gametype {} / {})",
					source->name, *s_globalDvarsCRC32, document.global_dvars_crc32,
					*s_lastGametypeIndex, static_cast<int>(document.gametype_count) - 1);
				return;
			}

			// same as other clients we have, match the "download path"
			*g_playlistsFetched = true;
			*s_playlistOutOfDate = false;
			UI_ValidatePlaylistNum();
			unk_ui_update_playlist_filters();
			requested = false;

			LOG("Component/Online/Playlists", INFO, "Loaded playlist '{}'!", source->name);
		}

		void update_playlist_source() {
			if (explicit_source || !native_ready()) {
				return;
			}

			const auto filename = get_playlist_filename();
			if (checked_filename == filename) {
				return;
			}

			checked_filename = filename;
			prepare_playlist(filename);
		}
	}

	bool playlist_available() {
		return native_ready() && source.has_value();
	}

	bool playlist_loaded() {
		return playlist_available() && !parse_failed && *g_playlistsFetched && !*s_playlistOutOfDate
			&& *s_globalDvarsCRC32 == source->document.global_dvars_crc32
			&& *s_lastGametypeIndex == static_cast<int>(source->document.gametype_count) - 1;
	}

	bool should_allow_online() {
		if (native_ready() && stats_initialized && game::Com_FrontEnd_IsInFrontEnd()) {
			update_playlist_source();
		}
		if (!playlist_available()) {
			return true;
		}

		if (!playlist_loaded() && !requested && !parse_failed && stats_initialized) {
			requested = true;
			scheduler::once<scheduler::pipeline::client>(load_playlist);
		}

		return playlist_loaded();
	}

	struct component final : client_component {
		bool should_enable() override {
			// we only enable this for versions that I attempted to verify, still need to do the rest later if possible
			return identification::game::is("1.07.1", "1.10.0", "1.14.0", "1.16.0", "1.24.1-ES", "1.27.0")
				&& identification::game::is_config(identification::game::Ship);
		}

		void post_load() override {
			auth::on_stats_init([] { stats_initialized = true; });
		}

		void find_signatures(memory::signature_store& batch) override {
			auto completion = "E8 ? ? ? ? C6 05 ? ? ? ? 01 C6 05 ? ? ? ? 00 E8 ? ? ? ? E8 ? ? ? ? 33 D2";

			if (identification::game::is_less_or_eq("1.14.0")) {
				completion = "E8 ? ? ? ? C6 05 ? ? ? ? 01 ? 88 ? ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 33 D2";

				batch.add(SETUP_POINTER(s_playlistOutOfDate), completion, SETUP_MOD(add(15).rip()));
			}
			else {
				batch.add(SETUP_POINTER(s_playlistOutOfDate), completion, SETUP_MOD(add(14).rip().add(1)));
			}

			batch.add(SETUP_POINTER(Playlist_ParsePlaylists), completion, SETUP_MOD(add(1).rip()));
			
			batch.add(SETUP_POINTER(g_playlistsFetched), completion, SETUP_MOD(add(7).rip().add(1)));

			batch.add(SETUP_POINTER(UI_ValidatePlaylistNum), completion, SETUP_MOD(add(20).rip()));

			batch.add(SETUP_POINTER(unk_ui_update_playlist_filters), completion, SETUP_MOD(add(25).rip()));

			if (identification::game::is_less_or_eq("1.10.0")) {
				batch.add(SETUP_POINTER(s_globalDvarsCRC32), "8B 1D ? ? ? ? 44 89 2D ? ? ? ? 74 ? F7 05", SETUP_MOD(add(2).rip()));
			}
			else {
				batch.add(SETUP_POINTER(s_globalDvarsCRC32), "8B 1D ? ? ? ? 44 89 3D ? ? ? ? 74 ? F7 05", SETUP_MOD(add(2).rip()));
			}

			if (identification::game::is("1.10.0")) {
				batch.add(SETUP_POINTER(s_lastGametypeIndex), "8B 15 ? ? ? ? FF C2 83 FA 60 0F 8D", SETUP_MOD(add(2).rip()));
			}
			else {
				batch.add(SETUP_POINTER(s_lastGametypeIndex), "44 8B 05 ? ? ? ? 41 FF C0 41 83 F8 60 0F 8D", SETUP_MOD(add(3).rip()));
			}
		}

		void post_unpack() override {
			if (!native_ready()) {
				LOG("Component/Online/Playlists", WARN, "Playlist loading is disabled, some functions weren't found.");
				return;
			}

			update_playlist_source();
			command::add("playlists_fetchFromLocal", [](const command::params_cl& params) {
				if (params.size() > 2) {
					LOG("Component/Online/Playlists", INFO, "Usage: playlists_fetchFromLocal <filename>");
					return;
				}

				if (!stats_initialized || !game::Com_FrontEnd_IsInFrontEnd()) {
					return;
				}

				const auto filename = params.size() == 2 ? std::string(params[1]) : get_playlist_filename();
				if (prepare_playlist(filename)) {
					explicit_source = params.size() == 2;
					checked_filename = filename;
					requested = true;
					load_playlist();
				}
			});
		}
	};
}

REGISTER_COMPONENT(online::playlists::component)
