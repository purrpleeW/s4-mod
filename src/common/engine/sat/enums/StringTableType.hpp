#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	enum class StringTableType : std::uint8_t {
		TYPE_NONE,
		TYPE_STRING,
		TYPE_INTEGER64,
		TYPE_FLOAT,
		TYPE_BOOLEAN,
		TYPE_XHASH_FNV,
		TYPE_XHASH_ASSET,
		TYPE_XHASH_DVAR,
		TYPE_INTEGER,
		TYPE_DEFAULT64,
		TYPE_UNK_10,
		TYPE_UNK_11
	};
}
