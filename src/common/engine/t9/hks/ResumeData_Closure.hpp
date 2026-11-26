#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/ResumeData_Header.hpp"

namespace t9::hks {
	class ResumeData_Closure {
	public:
		ResumeData_Header h_;	// 0x0000
	private:
		char pad_0004[0x0004];	// 0x0004
	public:
		HksClosure* closure_;	// 0x0008
		hksInt32 index_;		// 0x0010
	private:
		char pad_0014[0x0004];	// 0x0014
	};
	ENGINE_ASSERT_SZ(ResumeData_Closure, 0x0018);
}
