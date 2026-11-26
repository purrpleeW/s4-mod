#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/ResumeData_CClosure.hpp"
#include "engine/t9/hks/ResumeData_Closure.hpp"
#include "engine/t9/hks/ResumeData_State.hpp"
#include "engine/t9/hks/ResumeData_Table.hpp"
#include "engine/t9/hks/ResumeData_Userdata.hpp"

namespace t9::hks {
	union ResumeData_Entry {
		ResumeData_State state_;		// 0x0000
		ResumeData_Table hash_table_;	// 0x0000
		ResumeData_Closure closure_;	// 0x0000
		ResumeData_CClosure c_closure_;	// 0x0000
		ResumeData_Userdata userdata_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(ResumeData_Entry, 0x0020);
}
