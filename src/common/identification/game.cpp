#include "identification/game.hpp"
#include "identification/utility.hpp"
#include "memory/memory.hpp"
#include "utils/string.hpp"

#include "utils/safe_release.hpp"

#include <xxhash.h>

namespace identification::game {
	std::uint32_t get_library_xxh32_checksum(const utils::nt::library& library) {
		std::ifstream file(library.get_path(), std::ios::binary);
		if (!file) {
			return 0;
		}

		const auto _ = utils::safe_release([&file]{
			file.close();
		});

		auto* state = XXH32_createState();
		if (!state) {
			return 0;
		}
		XXH32_reset(state, 0);

		const auto _ = utils::safe_release([state]{
			XXH32_freeState(state);
		});

		constexpr auto streaming_buffer_size = 4 * (1024 * 1024);

		std::vector<char> buffer;
		buffer.reserve(streaming_buffer_size);

		while (file) {
			file.read(buffer.data(), streaming_buffer_size);
			std::streamsize bytes_read = file.gcount();
			if (bytes_read > 0) {
				XXH32_update(state, buffer.data(), bytes_read);
			}
		}

		return XXH32_digest(state);
	}

	std::uint32_t get_game_checksum() {
		static utils::nt::library game{};
		static std::uint32_t checksum = game.get_checksum();
		return checksum;
	}

	std::uint32_t get_game_xxh32_checksum() {
		static utils::nt::library game{};
		return get_library_xxh32_checksum(game);
	}

	std::uint32_t get_game_nt_timestamp() {
		static utils::nt::library game{};
		static std::uint32_t timestamp = game.get_nt_headers()->FileHeader.TimeDateStamp;
		return timestamp;
	}

	std::uint32_t get_target_checksum(iden_game_checksum_type type) {
		switch (type) {
		case iden_game_checksum_type::SlowBinaryTruncate:
			return get_game_checksum();
		case iden_game_checksum_type::XXH32:
			return get_game_xxh32_checksum();
		case iden_game_checksum_type::NtTimestamp:
			return get_game_nt_timestamp();
		case iden_game_checksum_type::ImplSpecific:
			return get_game_impl_specific();
		default:
			return UINT32_MAX;
		}
	}

	game_iden_target get_target_game_from_iden() {
		static auto iden = get_target_info_game();
		static game_iden_target target{};
		if (target.game_hash != 0x0) {
			return target;
		}

		for (const auto& it : iden.game_sets) {
			if (get_target_checksum(it.game_checksum_type) == it.game_hash) {
				target.client_name = iden.client_name;
				target.game_name = iden.game_name;
				target.game_version = it.game_version;
				target.game_mode = it.game_mode;
				target.game_platform = it.game_platform;
				target.game_config = it.game_config;
				target.game_hash = it.game_hash;
				target.game_checksum_type = it.game_checksum_type;
				return target;
			}

			continue;
		}

		return target;
	}

	game_iden_target get_target_game() {
		return get_target_game_from_iden();
	}

	std::string get_full_display_name() {
		static auto target = get_target_game_from_iden();

		std::string version = std::format(" v{}.{:02}.{}.{}",
			target.game_version[0], target.game_version[1], target.game_version[2], target.game_version[3]
		);

		std::string optional_config = "";
		if (target.game_config != iden_game_config::Ship) {
			std::string optional_config_name = "";
			switch (target.game_config) {
			case Replay:
				optional_config_name = "Replay Feature Branch";
				break;
			case Beta:
				optional_config_name = "Beta";
				break;
			case Alpha:
				optional_config_name = "Alpha";
				break;
			default:
				optional_config_name = "Unknown";
				break;
			}
			optional_config = std::format(" [{}]", optional_config_name);
		}

		std::string optional_mode = "";
		if (target.game_mode != iden_game_modes::AIO) {
			std::string optional_mode_name;
			switch (target.game_mode) {
			case iden_game_modes::MP:
				optional_mode_name = "Multiplayer";
				break;
			case iden_game_modes::SP:
				optional_mode_name = "Singleplayer";
				break;
			case iden_game_modes::ZM:
				optional_mode_name = "Zombies";
				break;
			case iden_game_modes::CDL:
				optional_mode_name = "E-Sports";
				break;
			default:
				optional_mode_name = "Unknown";
				break;
			}
			optional_mode = std::format(" ({})", optional_mode_name);
		}
		return std::format("Call of Duty®: {}{}{}{}", target.game_name, version, optional_config, optional_mode);
	}

	std::string get_version(bool show_delta) {
		static auto target = get_target_game_from_iden();

		std::string config = "";
		if (target.game_config != iden_game_config::Ship) {
			switch (target.game_config) {
			case Replay:
				config = "-replay";
				break;
			case Beta:
				config = "-beta";
				break;
			case Alpha:
				config = "-alpha";
				break;
			default:
				config = "";
				break;
			}
		}

		std::string mode = "";
		if (!show_delta) {
			switch (target.game_mode) {
			case iden_game_modes::MP:
				mode = "-MP";
				break;
			case iden_game_modes::SP:
				mode = "-SP";
				break;
			case iden_game_modes::ZM:
				mode = "-ZM";
				break;
			case iden_game_modes::CDL:
				mode = "-ES";
				break;
			default:
				mode = "";
				break;
			}
		}

		if (show_delta) {
			return std::format("{}.{:02}.{}.{}{}",
				target.game_version[0], target.game_version[1], target.game_version[2], target.game_version[3], config
			);
		}

		return std::format("{}.{:02}.{}{}{}",
			target.game_version[0], target.game_version[1], target.game_version[2], config, mode
		);
	}

	std::tuple<int, int, int> parse_str_version(const std::string& version) {
		std::string ver = version;
		if (ver.ends_with("-replay")) {
			ver = ver.substr(0, ver.size() - 7);
		}
		if (ver.ends_with("-beta")) {
			ver = ver.substr(0, ver.size() - 5);
		}
		if (ver.ends_with("-alpha")) {
			ver = ver.substr(0, ver.size() - 5);
		}
		std::istringstream iss(ver);
		std::string token;
		int major, minor, patch;

		std::getline(iss, token, '.');
		major = std::stoi(token);
		std::getline(iss, token, '.');
		minor = std::stoi(token);
		std::getline(iss, token, '.');
		patch = std::stoi(token);

		return { major, minor, patch };
	}
}
