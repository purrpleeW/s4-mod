#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"

namespace t9::hks {
	class StaticStringCache {
	public:
		HksObject objects_[42];	// 0x0000
	};
	ENGINE_ASSERT_SZ(StaticStringCache, 0x02A0);
}
