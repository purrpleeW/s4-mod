#include "common.hpp"

#include <ci/ci.hpp>
#include <loader/component_loader.hpp>
#include <memory/scanned_result.hpp>
#include <utils/binary_resource.hpp>
#include <utils/compression.hpp>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>
#include <utils/storage/storage_generic.hpp>
#include <utils/string.hpp>

#include "resource.h"

#define STEAM_EMU_DLL	"steam_api64_GSE.dll"
#define STEAM_APP_ID	"2000950"

namespace platforms::steam {
	namespace {
		utils::nt::library steam_api_emu{};

		const char* steam_deps[] = {
			"tier0_s64.dll", "vstdlib_s64.dll", "GameOverlayRenderer64.dll", "steamclient64.dll"
		};

		std::string get_resource_data(int resource_id, std::string name) {
			utils::binary_resource resource(resource_id, name);
			return resource.get_data();
		}

		// straight alterware paste ngl
		std::string get_steam_install_path() {
			static std::string install_path{};
			if (!install_path.empty()) {
				return install_path;
			}

			HKEY reg_key;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Valve\\Steam", 0, KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS) {
				char path[MAX_PATH]{};
				DWORD length = sizeof(path);
				RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, PTR_AS(BYTE*, path), &length);
				RegCloseKey(reg_key);

				install_path = path;
			}

			return install_path;
		}

		bool extract_resource(int resource_id, std::filesystem::path path) {
			if (std::filesystem::exists(path)) {
				return true;
			}

			auto resource_data = get_resource_data(resource_id, path.filename().string());
			std::ofstream out(path, std::ios::binary);
			if (out) {
				out.write(resource_data.data(), resource_data.size());
				out.close();
				return true;
			}

			LOG("Component/Steam", WARN, "Failed to open {} for writing.", path.filename().string());
			return false;
		}

		void* init_emulator_lib() {
			static class init_emulator_lib_ {
			public:
				init_emulator_lib_() {
					steam_api_emu = utils::nt::library::load("steam_api64_GSE.dll");
				}
			} init_emulator_lib;
			return &init_emulator_lib;
		}

		void ensure_emulator_lib() {
			return (void)init_emulator_lib();
		}

		bool extract_required_files() {
			if (auto steam_wd = get_steam_install_path(); !steam_wd.empty()) {
				LOG("Component/Steam", INFO, "Located existing Steam install at: {}", steam_wd);
			}

			auto cwd = std::filesystem::current_path();
			if (!extract_resource(STEAM_EMULATOR_LIB, cwd / STEAM_EMU_DLL)) {
				return false;
			}

			bool missing_any_deps = false;
			for (auto& dependency : steam_deps) {
				missing_any_deps = !std::filesystem::exists(cwd / dependency);
				if (missing_any_deps) {
					break;
				}
			}

			if (missing_any_deps) {
				auto steam_modules = utils::compression::zip::extract(get_resource_data(STEAM_EMULATOR_DEPS, "steam_modules.zip"));
				for (auto& [module_name, data] : steam_modules) {
					if (module_name.ends_with("/")) {
						continue;
					}

					auto file_path = cwd / module_name;
					if (std::filesystem::exists(file_path)) {
						continue;
					}

					std::ofstream out(file_path, std::ios::binary);
					if (out) {
						out.write(data.data(), data.size());
						out.close();
						continue;
					}

					LOG("Component/Steam", WARN, "Failed to open {} for writing.", module_name);
					return false;
				}
			}

			if (!std::filesystem::exists(cwd / "steam_settings")) {
				auto steam_settings = utils::compression::zip::extract(get_resource_data(STEAM_EMULATOR_SETTINGS, "steam_settings.zip"));
				for (auto& [file_name, data] : steam_settings) {
					if (file_name.ends_with("/")) {
						continue;
					}

					auto file_path = cwd / "steam_settings" / file_name;
					if (std::filesystem::exists(file_path)) {
						continue;
					}

					auto file_path_parent = file_path.parent_path();
					if (!std::filesystem::exists(file_path_parent)) {
						std::filesystem::create_directories(file_path_parent);
					}

					std::ofstream out(file_path, std::ios::binary);
					if (out) {
						out.write(data.data(), data.size());
						out.close();
						continue;
					}

					LOG("Component/Steam", WARN, "Failed to open {} for writing.", file_name);
					return false;
				}
			}

			return true;
		}

		utils::nt::library load_lib(const std::string& file) {
			if (auto steam_wd = get_steam_install_path(); !steam_wd.empty()) {
				auto lib_path = std::filesystem::path(steam_wd) / file;
				auto lib = utils::nt::library::load(lib_path);
				if (lib.is_valid()) {
					return lib;
				}
			}

			auto libs_folder = utils::storage::get_appdata_path() / "libs";
			std::filesystem::create_directories(libs_folder);

			auto lib_path = libs_folder / file;
			auto ref_lib_path = std::filesystem::current_path() / file;
			if (std::filesystem::exists(ref_lib_path) && !std::filesystem::exists(lib_path)) {
				std::filesystem::copy_file(ref_lib_path, lib_path, std::filesystem::copy_options::overwrite_existing);
			}

			return utils::nt::library::load(lib_path);
		}

		void patch_iat_entry(const std::string& name, void** entry_in = nullptr) {
			static utils::nt::library game{};

			ensure_emulator_lib();
			if (!steam_api_emu.is_valid()) {
				return;
			}

			auto emulated_proc = steam_api_emu.get_proc<void*>(name);
			if (emulated_proc == nullptr) {
				return;
			}

			auto entry = entry_in;
			if (entry == nullptr) {
				entry = game.get_iat_entry("steam_api64.dll", name);
				if (entry == nullptr) {
					return;
				}
			}

			utils::hook::set(entry, emulated_proc);
		}

		utils::nt::library attempt_load_lib(const std::string& name) {
			utils::nt::library(name).free();
			auto res = load_lib(name);
			if (!res.is_valid()) {
				LOG("Component/Steam", WARN, "Failed to load module: {}", name);
			}
			return res;
		}
	}

	struct component final : generic_component {
		bool should_enable() override {
			return identification::game::is_platform(identification::game::Steam);
		}

		void post_load() override {
			if (!extract_required_files()) {
				throw std::runtime_error("Failed to export Steam emulation files.");
			}

			utils::nt::library game{};
			auto iat_entries = game.get_all_iat_entries();
			CV_VM_DOLPHIN_START;
			for (auto& [mod_name, entry_map] : iat_entries) {
				if (strstr(mod_name.c_str(), "steam_api") == nullptr) {
					continue;
				}
				for (auto& [func_name, func_addr_ref] : entry_map) {
					patch_iat_entry(func_name, func_addr_ref);
				}
				break;
			}

			SetEnvironmentVariableA("SteamAppId", STEAM_APP_ID);

			std::vector<utils::nt::library> required_libraries{};
			required_libraries.push_back(steam_api_emu);
			for (auto& dependency : steam_deps) {
				required_libraries.push_back(attempt_load_lib(dependency));
			}

			for (auto& lib : required_libraries) {
				if (lib.is_valid()) {
					continue;
				}
				if (ci::is_testing()) {
					ci::end_test_run(-129 /* Platform Failure */);
				}
				else {
					throw std::runtime_error("Failed to find one or more Steam Emulator libraries. Cancelling execution.");
				}
				break;
			}
			CV_VM_DOLPHIN_END;
		}
	};
}

REGISTER_COMPONENT(platforms::steam::component)
