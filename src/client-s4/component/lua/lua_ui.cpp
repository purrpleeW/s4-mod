#include "common.hpp"
#include "component/lua/lua_ui.hpp"
#include "component/lua/lua_vm.hpp"

#include "component/key_catchers.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <component/command.hpp>
#include <utils/hook.hpp>

namespace lua::ui {
	namespace {
		functions::LUI_CoD_GetCurrentValidLocalClientT* LUI_CoD_GetCurrentValidLocalClient{};
		functions::LUI_Model_CreateModelFromPathT* LUI_Model_CreateModelFromPath{};
		functions::LUI_Model_GetGlobalModelT* LUI_Model_GetGlobalModel{};
		functions::LUI_Model_SetIntT* LUI_Model_SetInt{};
		functions::LUI_Model_SetStringT* LUI_Model_SetString{};
		functions::LUI_OpenMenuT* LUI_OpenMenu{};
	}

	s4::LocalClientNum_t get_client() {
		return LUI_CoD_GetCurrentValidLocalClient();
	}

	std::uint16_t create_model_from_path(std::uint16_t parent_node_index, const char* path) {
		return LUI_Model_CreateModelFromPath(parent_node_index, path);
	}

	std::uint16_t get_global_model() {
		return LUI_Model_GetGlobalModel();
	}

	void model_set_integer(std::uint16_t node_index, int new_value) {
		return LUI_Model_SetInt(node_index, new_value);
	}

	void model_set_string(std::uint16_t node_index, const char* new_value) {
		return LUI_Model_SetString(node_index, new_value);
	}

	void open_menu(const char* menu_name, bool is_immediate) {
		return LUI_OpenMenu(s4::LocalClientNum_t::LOCAL_CLIENT_0, menu_name, false, false, false, is_immediate);
	}

	void open_modal(const char* menu_name, bool is_immediate) {
		return LUI_OpenMenu(s4::LocalClientNum_t::LOCAL_CLIENT_0, menu_name, false, true, false, is_immediate);
	}

	void open_popup(const char* menu_name, bool is_immediate) {
		return LUI_OpenMenu(s4::LocalClientNum_t::LOCAL_CLIENT_0, menu_name, true, false, false, is_immediate);
	}

	struct component final : generic_component {
		public:
			void post_unpack() override {
				add_commands();
			}

		component_priority priority() const override {
			return component_priority::pointers;
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(LUI_CoD_GetCurrentValidLocalClient), "E8 ? ? ? ? 48 63 C8 8B D3", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(LUI_Model_CreateModelFromPath), "E8 ? ? ? ? 33 D2 66 41 89 87", SETUP_MOD(add(1).rip()));

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(LUI_Model_GetGlobalModel), "E8 ? ? ? ? 33 FF C7 44 24 ? ? ? ? ? C7 44 24", SETUP_MOD(add(1).rip()));
			}
			else {
				batch.add(SETUP_POINTER(LUI_Model_GetGlobalModel), "E8 ? ? ? ? 40 38 2D ? ? ? ? 44 0F B7 C0", SETUP_MOD(add(1).rip()));
			}

			batch.add(SETUP_POINTER(LUI_Model_SetInt), "48 89 74 24 ? 57 48 83 EC ? 0F B7 F9 8B F2");

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(LUI_Model_SetString), "E8 ?? ?? ?? ?? 48 8D 4C 24 ?? E8 ?? ?? ?? ?? BB ?? ?? ?? ?? E9", SETUP_MOD(add(1).rip()));
			} 
			else if (identification::game::is_less_or_eq("1.07.1")) {
				batch.add(SETUP_POINTER(LUI_Model_SetString), "40 55 56 48 83 EC ? 0F B7 E9");
			} 
			else {
				batch.add(SETUP_POINTER(LUI_Model_SetString), "40 56 41 56 48 83 EC ? 44 0F B7 F1");
			}

			batch.add(SETUP_POINTER(LUI_OpenMenu), "45 85 C0 74 ? 45 85 C9");

		}

	private:
		static void add_commands() {
			command::add("luiOpenMenu", [](const command::params_cl& params) {
				if (params.size() != 2) {
					return;
				}
				auto menu_name = std::string(params[1]);
				LUI_OpenMenu(s4::LocalClientNum_t::LOCAL_CLIENT_0, menu_name.c_str(), false, false, false, false);
				LOG("Component/Command", INFO, "Opened menu: {}", menu_name);
			});

			command::add("luiReload", [] {
				key_catchers::remove_catcher(-65);
				vm::quit_vm();
				vm::init_vm(game::Com_FrontEnd_IsInFrontEnd(), false);
				LOG("Component/Command", INFO, "Reloaded lua VM!");
			});
		}
	};
}

REGISTER_COMPONENT(lua::ui::component)
