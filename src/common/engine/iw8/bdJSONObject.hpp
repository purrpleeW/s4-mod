#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdHashingClass.hpp"
#include "engine/iw8/bdHashMap.hpp"
#include "engine/iw8/bdString.hpp"

namespace iw8 {
	class bdJSONObject {
	public:
		bdHashMap<bdString, bdJSONValue, bdHashingClass> map_;
	};
	ENGINE_ASSERT_SZ(bdJSONObject, 0x0020);
}
