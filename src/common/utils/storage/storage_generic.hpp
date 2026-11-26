#pragma once
#include "common_core.hpp"

#include "identification/game.hpp"
#include "identification/utility.hpp"
#include "utils/safe_release.hpp"
#include "utils/flags.hpp"

#include <expected>
#include "nlohmann/json.hpp"

#include "f2p_protection.hpp"

namespace utils::storage {
	namespace detail {
		struct storage_generic {
			virtual ~storage_generic() = default;
			virtual void init() {};

			virtual std::expected<void, std::string> set(const std::string& key, const std::string& value) = 0;
			virtual std::optional<std::string> get(const std::string& key) const = 0;

			virtual bool remove(const std::string& key) = 0;
			virtual bool contains(const std::string& key) const = 0;
			virtual std::expected<void, std::string> clear() = 0;

			virtual std::size_t size() const = 0;
			virtual std::vector<std::string> keys() const = 0;
		};
	}

	inline bool is_allowed_char(std::uint8_t c) {
		if (std::isalnum(c)) {
			return true;
		}
		static const char allowed[] = " ._-()[]{}+";
		return std::strchr(allowed, c) != nullptr;
	}

	inline std::string sanitize_alphanum(const std::string& input) {
		std::string out;
		out.reserve(input.size());
		for (std::uint8_t c : input) {
			if (!is_allowed_char(c)) {
				continue;
			}
			out.push_back(VAL_AS(char, c));
		}
		return out;
	}

	inline std::string get_client_folder() {
		static bool handled_dual = false;
		static std::string client_folder = identification::utility::get_client_name();
		if (!handled_dual) {
			if (utils::flags::has_flag("dual")) {
				client_folder += "-2";
			}
			handled_dual = true;
		}
		return client_folder;
	}

	inline std::filesystem::path get_game_mod_path() {
		crash_timebomb();
		static auto path = []() {
			auto dir = std::filesystem::current_path() / std::format(".{}", get_client_folder());
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_program_data_path() {
		crash_timebomb();
		static auto path = []() {
			PWSTR program_data;
			if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &program_data))) {
				throw std::runtime_error("Failed to read ProgramData path!");
			}

			auto _ = utils::safe_release([&program_data] {
				CoTaskMemFree(program_data);
			});

			auto dir = std::filesystem::path(program_data) / get_client_folder();
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_appdata_path() {
		crash_timebomb();
		static auto path = []() {
			PWSTR appdata;
			if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appdata))) {
				throw std::runtime_error("Failed to read LocalAppData path!");
			}

			auto _ = utils::safe_release([&appdata] {
				CoTaskMemFree(appdata);
			});

			auto dir = std::filesystem::path(appdata) / get_client_folder();
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_documents_path() {
		crash_timebomb();
		static auto path = []() {
			PWSTR documents;
			if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documents))) {
				throw std::runtime_error("Failed to read Documents path!");
			}

			auto _ = utils::safe_release([&documents] {
				CoTaskMemFree(documents);
			});

			auto dir = std::filesystem::path(documents) / get_client_folder();
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_game_configuration_path() {
		crash_timebomb();
		static auto path = []() {
			static std::unordered_map<std::uint8_t, std::string> platform_names = {
				{ identification::game::BattleNet, "bnet" },
				{ identification::game::MsStore, "gdk" },
				{ identification::game::Standalone, "standalone" },
				{ identification::game::Steam, "steam" }
			};

			std::string platform = "unknown";
			if (auto it = platform_names.find(identification::game::get_target_game().game_platform); it != platform_names.end()) {
				platform = it->second;
			}

			auto dir = get_documents_path() / "configuration" / platform / sanitize_alphanum(identification::utility::get_game_deltaless_version());
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_game_inventory_path() {
		crash_timebomb();
		static auto path = []() {
			auto dir = get_documents_path() / "inventory" / sanitize_alphanum(identification::utility::get_game_deltaless_version());
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}

	inline std::filesystem::path get_game_playercards_path() {
		crash_timebomb();
		static auto path = []() {
			auto dir = get_documents_path() / "playercards" / sanitize_alphanum(identification::utility::get_game_deltaless_version());
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
			return dir;
		}();
		return path;
	}
}
