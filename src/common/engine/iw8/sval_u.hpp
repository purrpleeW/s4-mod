#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/scr_string_t.hpp"

namespace iw8 {
	union sval_u {
		int type_;
		scr_string_t string_value_;
		std::uint32_t canonical_string_;
		std::uint32_t object_id_value_;
		std::uint32_t child_id_value_;
		float float_value_;
		int int_value_;
		sval_u* node_;
		std::uint32_t source_pos_value_;
		const char* code_pos_value_;
		const char* debug_string_;
		scr_block_t* block_;
	};
	ENGINE_ASSERT_SZ(sval_u, 0x0008);
}
