#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class GamerProfileDataType {
		TYPE_INVALID = 0,
		TYPE_BYTE = 1,
		TYPE_BOOL = 2,
		TYPE_SHORT = 3,
		TYPE_INT = 4,
		TYPE_FLOAT = 5,
		TYPE_STRING = 6,
		TYPE_BUFFER = 7,
		TYPE_FLAG = 8
	};
}
