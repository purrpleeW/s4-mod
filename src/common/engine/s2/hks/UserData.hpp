#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/HashTable.hpp"
#include "engine/s2/hks/Metatable.hpp"

namespace s2::hks {
	class UserData : public ChunkHeader {
		HashTable* env_;	// 0x0010
		Metatable* meta_;	// 0x0018
		char data_[8];		// 0x0020
	};
	ENGINE_ASSERT_SZ(UserData, 0x0028);
}
