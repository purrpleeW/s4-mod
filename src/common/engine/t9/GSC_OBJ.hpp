#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class GSC_OBJ {
	public:
		std::uint8_t magic_[8];				// 0x0000
		int crc_;							// 0x0008
	private:
		char pad_000C[0x0004];				// 0x000C
	public:
		std::uint64_t name_;				// 0x0010
		std::uint16_t string_count_;		// 0x0018
		std::uint16_t exports_count_;		// 0x001A
		std::uint16_t imports_count_;		// 0x001C
		std::uint16_t unk_001E;				// 0x001E
		std::uint16_t global_var_count_;	// 0x0020
		std::uint16_t unk_0022;				// 0x0022
		std::uint16_t includes_count_;		// 0x0024
		std::uint16_t unk_0026;				// 0x0026
		std::uint32_t unk_0028;				// 0x0028
		std::uint32_t cseg_offset_;			// 0x002C
		std::uint32_t string_offset_;		// 0x0030
		std::uint32_t includes_table_;		// 0x0034
		std::uint32_t exports_tables_;		// 0x0038
		std::uint32_t import_tables_;		// 0x003C
		std::uint32_t unk_0040;				// 0x0040
		std::uint32_t globalvar_offset_;	// 0x0044
		std::uint32_t file_size_;			// 0x0048
		std::uint32_t unk_004C;				// 0x004C
		std::uint32_t cseg_size_;			// 0x0050
		std::uint32_t unk_0054;				// 0x0054

		inline std::uint8_t get_vm() {
			return this->magic_[7];
		}
	};
	ENGINE_ASSERT_SZ(GSC_OBJ, 0x0058);
}
