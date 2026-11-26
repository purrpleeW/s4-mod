#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw8/Camo.hpp"
#include "identification/game.hpp"

namespace iw8 {
	bool camo_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("1.20.4");
		return use_v1;
	}

	std::uint8_t& Camo::get_blend_map_channels() {
		if (camo_use_v1()) {
			return this->v1_.blend_map_channels_;
		}
		return this->v2_.blend_map_channels_;
	}
}
#endif
