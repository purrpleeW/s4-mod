#include "common_core.hpp"
#include "proxies/proxy_base.hpp"

#include "identification/utility.hpp"
#include "identification/game.hpp"
#include "loader/component_loader.hpp"
#include "logger/logger.hpp"

#include "utils/flags.hpp"
#include "utils/hook.hpp"
#include "utils/properties.hpp"
#include "utils/string.hpp"

#include "utils/storage/storage_global.hpp"
#include "utils/storage/storage_local.hpp"
#include "utils/storage/storage_immutable.hpp"
#include "utils/storage/storage_user.hpp"

bool handle_process_runner() {
	const auto parent_proc_pid = utils::flags::get_flag<DWORD>("proc");
	if (parent_proc_pid == 0) {
		return false;
	}

	if (const auto process_handle = OpenProcess(SYNCHRONIZE, FALSE, parent_proc_pid)) {
		WaitForSingleObject(process_handle, INFINITE);
	}

	return true;
}

void proxy::on_dll_process_attach(HMODULE h_mod, bool server, bool avoid_alloc_console) {
	PROTECT_VOID(called);

	utils::nt::library game{};
	const auto game_name = utils::string::to_lower(game.get_name());

	const std::unordered_set<std::string> disallowed_module_names = {
		"bootstrapper.exe", "bootstrappercrashhandler.exe", "codbrokerinstaller.exe", "codcrashhandler.exe", "enrollaik.exe", "reportcodbug.exe",
		"sp22-codcrashhandler.exe", "vanguardcrashhandler.exe", "modern warfare launcher.exe", "crashlogsgenerator.exe", "dloguploader.exe"
	};

	if (disallowed_module_names.find(game_name) != disallowed_module_names.end()) {
		return;
	}

	if (!utils::nt::is_nt_10()) {
		const auto btn_press = MessageBoxA(nullptr,
			"This client has been launched on an unsupported operating system. "
			"To use this client, you must have Windows 10 or later installed.",
			"ZeroProxy", MB_OK | MB_ICONSTOP
		);

		if (btn_press) {
			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	utils::flags::parse_flags();

	utils::storage::immutable_storage.init();
	utils::storage::global_storage.init();
	utils::storage::local_storage.init();
	utils::storage::user_storage.init();

	if (handle_process_runner()) {
		exit(0);
	}

	DisableThreadLibraryCalls(h_mod);
	component_loader::ensure_load();

	logger::console::init(avoid_alloc_console);
	logger::console::set_full_title();

	auto appdata_path = utils::properties::get_appdata_path();
	if (!std::filesystem::exists(appdata_path)) {
		std::filesystem::create_directories(appdata_path);
	}

	if (!component_loader::activate(server)) {
		LOG("EntryPoint", ERROR, "Couldn't activate component loader.");
		exit(0);
	}

	if (!component_loader::post_load()) {
		LOG("EntryPoint", ERROR, "Couldn't execute post-load phase of component loader.");
		exit(0);
	}

	LOG("EntryPoint", INFO, "Successfully injected!");
}
