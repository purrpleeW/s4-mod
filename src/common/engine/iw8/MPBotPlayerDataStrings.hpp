#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class MPBotPlayerDisplayStrings {
	public:
		char display_name_[36];
  		char clan_tag_[11];
	};
	ENGINE_ASSERT_SZ(MPBotPlayerDisplayStrings, 0x2F);
}