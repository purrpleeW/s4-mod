#include "common.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::time_sync {
	namespace {
		bool live_storage_is_time_synced_stub() {
			*game::s_UTCSynced = true;
			return true;
		}
	}

	struct component final : generic_component {
		void post_unpack() override {
			utils::hook::jump(game::LiveStorage_IsTimeSynced, live_storage_is_time_synced_stub);
		}
	};
}

REGISTER_COMPONENT(patches::time_sync::component)
