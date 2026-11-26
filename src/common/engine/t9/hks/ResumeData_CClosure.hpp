#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/ResumeData_Header.hpp"

namespace t9::hks {
	class ResumeData_CClosure {
	public:
		ResumeData_Header h_;		// 0x0000
	private:
		char pad_0004[0x0004];		// 0x0004
	public:
		cclosure* c_closure_;		// 0x0008
		hksInt32 upvalue_index_;	// 0x0010
	private:
		char pad_0014[0x0004];		// 0x0014
	};
	ENGINE_ASSERT_SZ(ResumeData_CClosure, 0x0018);
}
