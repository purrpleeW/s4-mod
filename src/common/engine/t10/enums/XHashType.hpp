#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	enum class XHashType : std::uint8_t {
		HASH_TYPE_FNV = 0,
		HASH_TYPE_DVAR = 1,
		HASH_TYPE_ASSET = 3
	};
}
