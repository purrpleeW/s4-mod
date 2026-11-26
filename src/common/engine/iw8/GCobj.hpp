#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCcdata.hpp"
#include "engine/iw8/GCfunc.hpp"
#include "engine/iw8/GChead.hpp"
#include "engine/iw8/GCproto.hpp"
#include "engine/iw8/GCstr.hpp"
#include "engine/iw8/GCtab.hpp"
#include "engine/iw8/GCudata.hpp"
#include "engine/iw8/GCupval.hpp"
#include "engine/iw8/lua_State.hpp"

namespace iw8 {
	union GCobj {
		GChead gch_;	// 0x0000
		GCstr str_;		// 0x0000
		GCupval uv_;	// 0x0000
		lua_State th_;	// 0x0000
		GCproto pt_;	// 0x0000
		GCfunc fn_;		// 0x0000
		GCcdata cd_;	// 0x0000
		GCtab tab_;		// 0x0000
		GCudata ud_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GCobj, 0x0068);
}
