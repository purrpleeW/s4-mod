#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/BCInsLine.hpp"
#include "engine/iw8/GCstr.hpp"
#include "engine/iw8/SBuf.hpp"
#include "engine/iw8/TValue.hpp"
#include "engine/iw8/VarInfo.hpp"

namespace iw8 {
	class LexState {
	public:
		FuncState* fs_;			// 0x0000
		lua_State* l_;			// 0x0008
		TValue tok_val_;		// 0x0010
		TValue look_ahead_val_;	// 0x0018
		const char* p;			// 0x0020
		const char* pe;			// 0x0028
		LexChar c_;				// 0x0030
		LexToken tok_;			// 0x0034
		LexToken look_ahead_;	// 0x0038
	private:
		char pad_003C[0x0004];	// 0x003C
	public:
		SBuf sb_;				// 0x0040
		lua_Reader r_func_;		// 0x0060
		void* r_data_;			// 0x0068
		BCLine line_number_;	// 0x0070
		BCLine last_line_;		// 0x0074
		GCstr* chunk_name_;		// 0x0078
		const char* chunk_arg;	// 0x0080
		const char* mode_;		// 0x0088
		VarInfo* v_stack_;		// 0x0090
		MSize size_v_stack_;	// 0x0098
		MSize v_top_;			// 0x009C
		BCInsLine* bc_stack_;	// 0x00A0
		MSize size_bc_stack_;	// 0x00A8
		std::uint32_t level_;	// 0x00AC
	};
	ENGINE_ASSERT_SZ(LexState, 0x00B0);
}
