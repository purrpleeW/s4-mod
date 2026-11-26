#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	enum class DvarType : std::uint8_t {
		DVAR_TYPE_BOOL = 0,
		DVAR_TYPE_FLOAT = 1,
		DVAR_TYPE_FLOAT_2 = 2,
		DVAR_TYPE_FLOAT_3 = 3,
		DVAR_TYPE_FLOAT_4 = 4,
		DVAR_TYPE_INT = 5,
		DVAR_TYPE_UINT = 6,
		DVAR_TYPE_INT64 = 7,
		DVAR_TYPE_UINT64 = 8,
		DVAR_TYPE_ENUM = 9,
		DVAR_TYPE_STRING = 10,
		DVAR_TYPE_COLOR = 11,
		DVAR_TYPE_FLOAT_3_COLOR = 12,
		DVAR_TYPE_XHASH = 13,
		DVAR_TYPE_XHASH_ASSET = 14,
		DVAR_TYPE_COUNT = 15
	};
}
