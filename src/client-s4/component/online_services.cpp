#include "common.hpp"
#include "game/game.hpp"

#include "identification/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace online_services {
	namespace {
		functions::unk_OnlineServicesFuncT* unk_OnlineServicesFunc{};

		utils::hook::detour unk_online_services_func_hook;

		/*
		bool unk_online_services_func_stub(void* a1) {
			if (a1 == nullptr) {
				return true;
			}
			return unk_online_services_func_hook.invoke<bool>(a1);
		}
		*/
		bool unk_online_services_func_stub(void* a1) {
			return true;
		}
	}

	struct component final : generic_component {
		void find_signatures(memory::signature_store& batch) override {
			if (!identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(unk_OnlineServicesFunc), "48 89 5C 24 ? 48 89 7C 24 ? 44 8B 59 ? 45 33 C0");
			}
		}

		void post_unpack() override {
			if (unk_OnlineServicesFunc) {
				unk_online_services_func_hook.create(unk_OnlineServicesFunc, unk_online_services_func_stub);
			}
		}
	};
}

REGISTER_COMPONENT(online_services::component)
