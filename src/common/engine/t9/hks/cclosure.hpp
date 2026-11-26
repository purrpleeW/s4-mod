#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"

namespace t9::hks {
	class cclosure : public ChunkHeader {
	public:
		lua_CFunction function_;	// 0x0010
		hks::HashTable* env_;		// 0x0018
		hksShort16 num_upvalues_;	// 0x0020
		hksShort16 flags_;			// 0x0022
	private:
		char pad_0024[0x0004];		// 0x0024
	public:
		hks::InternString* name_;	// 0x0028
		HksObject upvalues_[1];		// 0x0030
	};
	ENGINE_ASSERT_SZ(cclosure, 0x0040);
}
