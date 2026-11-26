#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	template <typename T>
	class bdReference {
		T* ptr_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdReference<void>, 0x0008);
}
