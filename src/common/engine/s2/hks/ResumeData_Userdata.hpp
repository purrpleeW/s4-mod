#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/ResumeData_Header.hpp"

namespace s2::hks {
	class ResumeData_Userdata {
	public:
		ResumeData_Header h_;	// 0x0000
	private:
		char pad_0004[0x0004];	// 0x0004
	public:
		UserData* data_;		// 0x0008
	};
	ENGINE_ASSERT_SZ(ResumeData_Userdata, 0x0010);
}
