#include "common.hpp"
#include "game/game.hpp"

#include "component/assets.hpp"
#include "component/lua/lua_vm.hpp"
#include "component/online/playlists.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

#include "resource.h"

namespace assets::lua_file {
	namespace {
		struct lua_replace_info {
			std::string target_;
			int resource_id_;
			std::function<bool()> condition_ = [] { return true; };
		};

		functions::luaL_loadfileT* luaL_loadfile{};
		utils::hook::detour lua_l_load_file_hook;

		std::mutex reading_mutex{};
		lua_replace_info lua_replace_from_resource[] = {
			{ "ui/frontEnd/mp/QuickPlayButton.lua", LUA_REPLACE_ui_frontEnd_mp_QuickPlayButton, [] {
				return identification::game::is_config(identification::game::Ship) && !online::playlists::playlist_available();
			} },
		};

		int lua_l_load_file_stub(s4::lua_State* s, const char* file_name) {
			auto res = lua_l_load_file_hook.invoke<int>(s, file_name);
			if (file_name != nullptr) {
				std::string internal_file_name = std::format("rpl:{}", file_name);
				std::filesystem::path path = assets::get_content_path(
					assets::get_content_type_name(s4::XAssetType::ASSET_TYPE_LUAFILE), file_name
				);

				if (std::filesystem::exists(path)) {
					std::ifstream file(path, std::ios::binary | std::ios::ate);
					if (!file) {
						LOG("Component/Assets/LuaFile", WARN, "Couldn't open file: {}", file_name);
						return res;
					}

					std::streamsize size = file.tellg();
					file.seekg(0, std::ios::beg);

					std::string buffer(size, '\0');
					if (!file.read(buffer.data(), size)) {
						LOG("Component/Assets/LuaFile", WARN, "Failed to read file: {}", file_name);
						return res;
					}

					if (lua::vm::load_buffer(s, buffer.data(), buffer.size(), internal_file_name.c_str()) != 0) {
						LOG("Component/Assets/LuaFile", WARN, "luaL_loadbuffer failed for {}", file_name);
					}
					else {
						return res;
					}
				}

				for (const auto& replace_info : lua_replace_from_resource) {
					if (replace_info.target_ != file_name || !replace_info.condition_()) {
						continue;
					}

					LOG("Component/Assets/LuaFile", INFO, "Loading internal Lua: {}", file_name);
					auto buffer = utils::nt::load_resource(replace_info.resource_id_);
					if (lua::vm::load_buffer(s, buffer.data(), buffer.size(), internal_file_name.c_str()) != 0) {
						LOG("Component/Assets/LuaFile", WARN, "luaL_loadbuffer failed for {}", replace_info.target_);
					}
				}
			}
			return res;
		}
	}

	struct component final : generic_component {
		void post_load() override {
			static auto asset_type = s4::XAssetType::ASSET_TYPE_LUAFILE;

			assets::register_callback(asset_type, [](const char* _, s4::XAssetHeader& res) {
				static auto generic_lua_xasset = new s4::LuaFile();
				if (res.lua_file_ == nullptr) {
					res.lua_file_ = generic_lua_xasset;
				}
			});
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(luaL_loadfile), "40 53 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B 41 ? 48 8B FA");
		}

		void post_unpack() override {
			lua_l_load_file_hook.create(luaL_loadfile, lua_l_load_file_stub);
		}
	};
}

REGISTER_COMPONENT(assets::lua_file::component)