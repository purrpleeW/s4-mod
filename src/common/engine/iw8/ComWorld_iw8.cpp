#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw8/ComWorld.hpp"
#include "identification/game.hpp"

namespace iw8 {
	bool com_world_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("1.03.0");
		return use_v1;
	}

	ComChangeListInfo& ComWorld::get_change_list_info() {
		if (com_world_use_v1()) {
			return this->v1_.change_list_info_;
		}
		return this->v2_.change_list_info_;
	}
}
#endif
