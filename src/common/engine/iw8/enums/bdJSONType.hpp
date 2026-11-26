#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class bdJSONType {
		BD_JSON_NULL = 0,
		BD_JSON_STRING = 1,
		BD_JSON_NUMBER = 2,
		BD_JSON_BOOLEAN = 3,
		BD_JSON_ARRAY = 4,
		BD_JSON_OBJECT = 5
	};
}
