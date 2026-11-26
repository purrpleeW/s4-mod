#include "common.hpp"
#include "component/filesystem.hpp"

#include "game/game.hpp"

#include <loader/component_interface.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/storage/storage_generic.hpp>

namespace filesystem {
	namespace {
		functions::FS_StartupT* FS_Startup;

		utils::hook::detour fs_startup_hook;

		bool initialized = false;

		std::vector<std::function<void()>>& get_fs_init_callbacks() {
			static std::vector<std::function<void()>> callbacks{};
			return callbacks;
		}

		std::deque<std::filesystem::path>& get_search_paths_internal() {
			static std::deque<std::filesystem::path> search_paths{};
			return search_paths;
		}

		void fs_startup_stub(const char* name) {
			LOG("Component/FileSystem", INFO, "Startup");

			initialized = true;

			filesystem::register_path(utils::storage::get_game_mod_path());
			filesystem::register_path(L"devraw");
			filesystem::register_path(L"devraw_shared");
			filesystem::register_path(L"raw_shared");
			filesystem::register_path(L"raw");
			filesystem::register_path(L"main");

			for (const auto& callback : get_fs_init_callbacks()) {
				callback();
			}

			fs_startup_hook.invoke<void>(name);
		}

		std::vector<std::filesystem::path> get_paths(const std::filesystem::path& path) {
			std::vector<std::filesystem::path> paths{};
			paths.push_back(path);
			return paths;
		}

		bool can_insert_path(const std::filesystem::path& path) {
			for (const auto& path_ : get_search_paths_internal()) {
				if (path_ == path) {
					return false;
				}
			}

			return true;
		}
	}

	std::string read_file(const std::string& path) {
		for (const auto& search_path : get_search_paths_internal()) {
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string())) {
				return utils::io::read_file(path_.generic_string());
			}
		}

		return {};
	}

	bool read_file(const std::string& path, std::string* data, std::string* real_path) {
		for (const auto& search_path : get_search_paths_internal()) {
			const auto path_ = search_path / path;
			if (utils::io::read_file(path_.generic_string(), data)) {
				if (real_path != nullptr) {
					*real_path = path_.generic_string();
				}

				return true;
			}
		}

		return false;
	}

	bool find_file(const std::string& path, std::string* real_path) {
		for (const auto& search_path : get_search_paths_internal()) {
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string())) {
				*real_path = path_.generic_string();
				return true;
			}
		}

		return false;
	}

	bool exists(const std::string& path) {
		for (const auto& search_path : get_search_paths_internal()) {
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string())) {
				return true;
			}
		}

		return false;
	}

	void register_path(const std::filesystem::path& path) {
		if (!initialized) {
			return;
		}

		const auto paths = get_paths(path);
		for (const auto& path_ : paths) {
			if (can_insert_path(path_)) {
				LOG("Component/FileSystem", INFO, "Registering path '{}'", path_.generic_string());
				get_search_paths_internal().push_front(path_);
			}
		}
	}

	void unregister_path(const std::filesystem::path& path) {
		if (!initialized) {
			return;
		}

		const auto paths = get_paths(path);
		for (const auto& path_ : paths) {
			auto& search_paths = get_search_paths_internal();
			for (auto i = search_paths.begin(); i != search_paths.end();) {
				if (*i == path_) {
					LOG("Component/FileSystem", INFO, "Unregistering path '{}'", path_.generic_string());
					i = search_paths.erase(i);
				}
				else {
					++i;
				}
			}
		}
	}

	std::vector<std::string> get_search_paths() {
		std::vector<std::string> paths{};

		for (const auto& path : get_search_paths_internal()) {
			paths.push_back(path.generic_string());
		}

		return paths;
	}

	std::vector<std::string> get_search_paths_rev() {
		std::vector<std::string> paths{};
		const auto& search_paths = get_search_paths_internal();

		for (auto i = search_paths.rbegin(); i != search_paths.rend(); ++i) {
			paths.push_back(i->generic_string());
		}

		return paths;
	}

	void on_fs_init(std::function<void()> callback) {
		get_fs_init_callbacks().push_back(callback);
	}

	class component final : public generic_component {
	public:
		void find_signatures(memory::signature_store& batch) override {
			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(FS_Startup), "E8 ? ? ? ? 33 C9 E8 ? ? ? ? 41 B8 ? ? ? ? 48 8D 15", SETUP_MOD(add(1).rip()));
			}
			else if (identification::game::is_less_or_eq("1.10.0")) {
				batch.add(SETUP_POINTER(FS_Startup), "E8 ?? ?? ?? ?? 33 C9 E8 ?? ?? ?? ?? 44 8D 43", SETUP_MOD(add(1).rip()));
			}
			else {
				batch.add(SETUP_POINTER(FS_Startup), "E8 ? ? ? ? 33 C9 E8 ? ? ? ? 41 B8 ? ? ? ? 48 8D 15", SETUP_MOD(add(1).rip()));
			}
		}

		void post_unpack() override {
			if (FS_Startup == nullptr) {
				return;
			}
			fs_startup_hook.create(FS_Startup, fs_startup_stub);
		}
	};
}

REGISTER_COMPONENT(filesystem::component)
