#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/VariableStackBuffer.hpp"
#include "engine/s4/enums/scr_string_t.hpp"

namespace s4 {
	union VariableUnion {
		int int_value_;							// 0x0000
		std::uint32_t uint_value_;				// 0x0000
		float float_value_;						// 0x0000
		scr_string_t string_value_;				// 0x0000
		const float* vector_value_;				// 0x0000
		const char* code_pos_value_;			// 0x0000
		std::uint64_t script_code_pos_value_;	// 0x0000
		std::uint32_t pointer_value_;			// 0x0000
		VariableStackBuffer* stack_value_;		// 0x0000
		std::uint32_t entity_offset_;			// 0x0000
	};
}
