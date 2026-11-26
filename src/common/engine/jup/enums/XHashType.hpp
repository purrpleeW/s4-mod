#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	enum class XHashType : std::uint8_t {
		HASH_TYPE_FNV,
		HASH_TYPE_DVAR,
		HASH_TYPE_ASSET,
		HASH_TYPE_X32,
		HASH_TYPE_COUNT,
		HASH_TYPE_INVALID = HASH_TYPE_COUNT
	};
}
