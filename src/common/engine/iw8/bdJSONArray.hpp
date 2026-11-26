#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdArray.hpp"

namespace iw8 {
	class bdJSONArray {
	public:
		bdArray<bdJSONValue> array_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdJSONArray, 0x0010);
}
