#include "common.hpp"
#include "component/assets.hpp"

#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <atomic>

namespace assets {
	namespace {
		functions::DB_LoadXFileT* DB_LoadXFile{};

		utils::hook::detour db_find_x_asset_header_hook;
		utils::hook::detour db_load_x_file_hook;

		std::vector<std::function<AssetLoadCallback>> asset_load_callbacks{};
		std::map<s4::XAssetType, std::function<AssetReplacementCallback>> asset_replace_callbacks{};
		std::map<s4::XAssetType, std::map<std::string, s4::XAssetHeader>> allocated_assets{};
		std::mutex reading_mutex{};
		std::atomic_uint fastfile_load_depth{};
		struct fastfile_load_scope {
			fastfile_load_scope() { fastfile_load_depth.fetch_add(1); }
			~fastfile_load_scope() { fastfile_load_depth.fetch_sub(1); }
		};

		s4::XAssetHeader db_find_x_asset_header_stub(s4::XAssetType type, const char* given_name, int allow_create_default) {
			auto res = find_x_asset_header(type, given_name, allow_create_default);

			if (given_name != nullptr && strcmp(given_name, "*") != 0) {
				auto it = asset_replace_callbacks.find(type);

				if (it != asset_replace_callbacks.end()) {
					std::lock_guard _(reading_mutex);
					it->second(given_name, res);
				}
			}

			for (auto& callback : asset_load_callbacks) {
				callback(type, given_name, allow_create_default, res);
			}

			return res;
		}

		int db_load_x_file_stub(const char* zone_name, s4::XZoneMemory* zone_mem, s4::XAssetList* asset_list, std::uint32_t zone_flags, bool was_paused,
			s4::DB_FastFileFailureMode failure_mode, s4::DB_AuthSignature* out_signature)
		{
			const fastfile_load_scope loading;
			LOG("Component/Assets", INFO, "Loading fastfile {}...", zone_name ? zone_name : "<null>");

			int res = db_load_x_file_hook.invoke<int>(zone_name, zone_mem, asset_list, zone_flags, was_paused, failure_mode, out_signature);
			if (res != 0) {
				LOG("Component/Assets", WARN, "Loaded fastfile {} with non-zero code {}.", zone_name ? zone_name : "<null>", res);
			}
			return res;
		}

		int db_load_x_file_legacy_stub(const char* zone_name, s4::DB_FastFileFailureMode failure_mode, std::uint64_t a3, std::uint64_t a4, bool was_paused) {
			const fastfile_load_scope loading;
			LOG("Component/Assets", INFO, "Loading fastfile {}...", zone_name ? zone_name : "<null>");

			int res =  db_load_x_file_hook.invoke<int>(zone_name, failure_mode, a3, a4, was_paused);
			if (res != 0) {
				LOG("Component/Assets", WARN, "Loaded fastfile {} with non-zero code {}.", zone_name ? zone_name : "<null>", res);
			}
			return res;
		}
	}

	s4::XAssetHeader* get_allocated_asset(s4::XAssetType type, const std::string& name) {
		auto [it_type, _x] = allocated_assets.try_emplace(type);
		auto [it_asset, _y] = it_type->second.try_emplace(name);
		return &it_asset->second;
	}

	s4::XAssetHeader find_x_asset_header(s4::XAssetType type, const char* given_name, int allow_create_default) {
		return db_find_x_asset_header_hook.invoke<s4::XAssetHeader>(type, given_name, allow_create_default);
	}

	bool is_loading_fastfile() {
		return fastfile_load_depth.load() != 0;
	}

	std::filesystem::path get_content_path(const std::string& type, const std::string& file) {
		auto path = utils::properties::get_game_mod_path() / "asset_replacer";
		if (!type.empty()) {
			path /= type;
			if (!file.empty()) {
				path /= file;
			}
		}
		return path;
	}

	std::string get_clean_script_file_name(const std::string& script_file_name, bool compiled) {
		auto cleaned = /*gsc::get_real_script_name*/(script_file_name);
		if (!utils::string::ends_with(script_file_name, ".gsc")) {
			cleaned.append(".gsc");
		}
		if (compiled) {
			cleaned.append("bin");
		}
		return cleaned;
	}

	void on_asset_load(std::function<AssetLoadCallback> callback) {
		asset_load_callbacks.push_back(callback);
	}

	void register_callback(s4::XAssetType type, const std::function<AssetReplacementCallback>& callback) {
		asset_replace_callbacks[type] = callback;
	}

	void write_default_asset_file(std::filesystem::path path, std::function<void(std::ofstream&)> callback) {
		std::filesystem::path dir = path.parent_path();
		std::string name = path.filename().string();

		if (!std::filesystem::exists(dir)) {
			std::filesystem::create_directories(dir);
		}

		std::ofstream file(path, std::ios::binary);
		if (!file) {
			LOG("Component/Assets", WARN, "Couldn't open file for writing default asset: {}", name);
			return;
		}

		callback(file);
		if (!file) {
			LOG("Component/Assets", WARN, "Couldn't write default asset to file: {}", name);
		}

		file.close();
	}

	struct component final : client_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(DB_LoadXFile), "E8 ? ? ? ? 8B F8 44 38 AB", SETUP_MOD(add(1).rip()));
		}

		void post_unpack() override {
			db_find_x_asset_header_hook.create(game::DB_FindXAssetHeader, db_find_x_asset_header_stub);
			db_load_x_file_hook.create(DB_LoadXFile, db_load_x_file_stub);
		}
	};
}

REGISTER_COMPONENT(assets::component)
