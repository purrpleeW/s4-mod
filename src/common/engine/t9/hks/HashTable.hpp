#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"
#include "engine/t9/hks/Metatable.hpp"

namespace t9::hks {
	class HashTable : public ChunkHeader {
		class Node {
		public:
			HksRegister key_;		// 0x0000
			HksRegister value_;		// 0x0010
		};

		Metatable* meta_;			// 0x0010
		hksUint32 mask_;			// 0x0018
	private:
		char pad_001C[0x0004];		// 0x001C
	public:
		Node* hash_part_;			// 0x0020
		HksRegister* array_part_;	// 0x0028
		hksUint32 array_size_;		// 0x0030
	private:
		char pad_0034[0x0004];		// 0x0034
	public:
		Node* free_node_;			// 0x0038
	};
	ENGINE_ASSERT_SZ(HashTable, 0x0040);
}
