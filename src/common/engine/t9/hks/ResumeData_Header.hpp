#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/enums/HksObjectType.hpp"

namespace t9::hks {
	class ResumeData_Header {
	public:
		HksObjectType type_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(ResumeData_Header, 0x0004);
}
