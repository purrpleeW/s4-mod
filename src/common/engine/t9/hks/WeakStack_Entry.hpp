#pragma once
#include "engine/engine_common.hpp"

namespace t9::hks {
	class WeakStack_Entry {
	public:
		hksInt32 weakness;		// 0x0000
	private:
		char pad_0004[0x0004];	// 0x0004
	public:
		hks::HashTable* table;	// 0x0008
	};
	ENGINE_ASSERT_SZ(WeakStack_Entry, 0x0010);
}
