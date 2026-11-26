#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdHashingClass {
	private:
		char pad_0000[0x0001];	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdHashingClass, 0x0001);
}
