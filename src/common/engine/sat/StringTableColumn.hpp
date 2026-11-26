#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/StringTableValueSOAUnion.hpp"
#include "engine/sat/enums/StringTableType.hpp"

namespace sat {
	class StringTableColumn {
	public:
		StringTableType type_;				// 0x0000
	private:
		char pad_0001[0x000F];				// 0x0001
	public:
		std::uint16_t* cell_indices_;		// 0x0010
	private:
		char pad_0018[0x0008];				// 0x0018
	public:
		StringTableValueSOAUnion values_;	// 0x0020
	};
	ENGINE_ASSERT_SZ(StringTableColumn, 0x0028);
}
