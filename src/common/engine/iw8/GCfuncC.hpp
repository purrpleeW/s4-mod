#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"
#include "engine/iw8/lua_State.hpp"
#include "engine/iw8/MRef.hpp"
#include "engine/iw8/TValue.hpp"

namespace iw8 {
	class GCfuncC {
	public:
		GCRef next_gc_;				// 0x0000
		std::uint8_t marked_;		// 0x0008
		std::uint8_t gct_;			// 0x0009
		std::uint8_t ff_id_;		// 0x000A
		std::uint8_t nup_values_;	// 0x000B
	private:
		char pad_000C[0x0004];		// 0x000C
	public:
		GCRef env_;					// 0x0010
		GCRef gc_list_;				// 0x0018
		MRef pc_;					// 0x0020
		int(*f_)(lua_State*);		// 0x0028
		TValue upvalue_[1];			// 0x0030
	};
	ENGINE_ASSERT_SZ(GCfuncC, 0x0038);
}
