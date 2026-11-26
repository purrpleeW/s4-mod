#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/DDLEnum.hpp"
#include "engine/s4/DDLStruct.hpp"

namespace s4 {
	class DDLDef {
	public:
		char* name_;				// 0x0000
		std::uint16_t version_;		// 0x0008
	private:
		char pad_000A[0x0006];		// 0x000A
	public:
		std::uint64_t guid_seed_;	// 0x0010
		std::uint64_t guid_;		// 0x0018
		int bit_size_;				// 0x0020
		int byte_size_;				// 0x0024
		DDLStruct* struct_list_;	// 0x0028
		int struct_count_;			// 0x0030
	private:
		char pad_0034[0x0004];		// 0x0034
	public:
		DDLEnum* enum_list_;		// 0x0038
		int enum_count_;			// 0x0040
	private:
		char pad_0044[0x0004];		// 0x0044
	public:
		DDLDef* next_;				// 0x0048
		int header_bit_size_;		// 0x0050
		int header_byte_size_;		// 0x0054
		bool padding_used_;			// 0x0058
		bool minimal_header_;		// 0x0059
	private:
		char pad_005A[0x0006];		// 0x005A
	};
	ENGINE_ASSERT_SZ(DDLDef, 0x0060);
}
