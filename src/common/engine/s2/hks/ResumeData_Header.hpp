#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/enums/HksObjectType.hpp"

namespace s2::hks {
	class ResumeData_Header {
	public:
		HksObjectType type_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(ResumeData_Header, 0x0004);
}
