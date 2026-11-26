#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::dw_fence {
	namespace {
		functions::Live_IsDemonwareFailedT* Live_IsDemonwareFailed{};

		bool live_is_demonware_failed_stub() {
			return true;
		}
	}

	struct component final : generic_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Live_IsDemonwareFailed), "E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 8B 54 24 ? 48 8B CB", SETUP_MOD(add(1).rip()));
		}

		bool should_activate() override {
			return Live_IsDemonwareFailed != nullptr;
		}

		void post_unpack() override {
			utils::hook::jump(Live_IsDemonwareFailed, live_is_demonware_failed_stub);
		}
	};
}

REGISTER_COMPONENT(patches::dw_fence::component)
