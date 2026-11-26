#include "common.hpp"
#include "game/game.hpp"

#include "component/lua/lua_hook.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

#include <magic_enum/magic_enum.hpp>

namespace patches::scenarios {
	namespace {
		functions::Engine_IsScenarioGroupInstalledT* Engine_IsScenarioGroupInstalled{};
		utils::hook::detour engine_is_scenario_group_installed_hook{};

		s4::Scenario** s_scenarioList{};
		s4::ScenarioGroup** s_scenarioGroupList{};
		int* s_scenarioCount{};
		int* s_scenarioGroupCount{};

		bool engine_is_scenario_group_installed_stub(s4::XHash xhash) {
			if (*s_scenarioList && *s_scenarioCount > 0) {
				const auto early_layout = identification::game::is("1.07.1");
				for (auto ix = 0; ix < *s_scenarioCount; ix++) {
					auto& scenario = (*s_scenarioList)[ix];
					if (early_layout) {
						// 1.07.1 checks +0x20 instead of the shared layouts +0x24, because of course it fucking does
						CLASS_VALUE_AT(&scenario, 0x20, s4::ScenarioInstallState) = s4::ScenarioInstallState::Installed;
					} else {
						scenario.install_state_ = s4::ScenarioInstallState::Installed;
					}
				}
			}

			return engine_is_scenario_group_installed_hook.invoke<bool>(xhash);
		}
	}

	struct component final : generic_component {
		//~ So for some context. Engine_IsScenarioGroupInstalled(XHash) calls
		//~		Scenarios_IsScenarioGroupInstalled(ScenarioGroup*) with a value of 0
		//~		if scenarioIndex, which increments as it's looping through a flat
		//~		array of installed scenarios at 0x118f0348 (1.26.1 Steam), is greater
		//~		or equal to s_scenarioGroupCount. Why? I have no idea but
		//~		Scenarios_Platform_IsScenarioInstalled also makes no attempt to check
		//~		if the the pointer is null, which they seemed to have subsequently fixed
		//~		on MWII onwards (as seen in WZM 11.5 iOS Symbols - 0xcd922c).
		//~
		//~ This is why in vg/v1.0.0 we hooked Scenarios_IsScenarioGroupInstalled(ScenarioGroup*)
		//~		after setting the "unk_SPLaunchVar", now known to be s_scenarioGroupCount, to 0
		//~
		//~ In the new method, we manipulate the s_scenarioList and s_scenarioGroupList by hooking
		//~		Engine_IsScenarioGroupInstalled, then looping through the list and setting the right
		//~		values and then returning calling original, as there is some stat stuff in there I'm
		//~		not going to touch today, if it blows up i'll think about it.
		//~
		//~ (luxploit)

		bool should_enable() override {
			return !identification::game::is_config(identification::game::Beta);
		}

		void post_load() override {
			lua::hook::set("Engine.IsGameModeAllowed", true);
			//lua::hook::set("Engine.IsScenarioGroupInstalled", true);
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Engine_IsScenarioGroupInstalled), "E8 ? ? ? ? 84 C0 74 ? BA ? ? ? ? 0F B6 CA", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(s_scenarioGroupCount), "44 8B 0D ? ? ? ? 45 33 C0 45 85 C9 0F 84", [](memory::scanned_result<> res) {
				auto offset = res.add(3).rip();

				offset.sub(0x04).apply(&s_scenarioCount);
				offset.add(0x0C).apply(&s_scenarioList);
				offset.add(0x14).apply(&s_scenarioGroupList);

				return offset;
			});
		}

		bool should_activate() override {
			return Engine_IsScenarioGroupInstalled && s_scenarioGroupCount
				&& s_scenarioCount  && s_scenarioGroupList && s_scenarioList
			;
		}

		void post_unpack() override {
			engine_is_scenario_group_installed_hook.create(Engine_IsScenarioGroupInstalled, engine_is_scenario_group_installed_stub);
		}
	};
}

REGISTER_COMPONENT(patches::scenarios::component)
