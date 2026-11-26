#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"

namespace t9::hks {
	class UpValue : public ChunkHeader {
	public:
		HksObject storage_;		// 0x0010
		HksObject* loc_;		// 0x0020
		hks::UpValue* next_;	// 0x0028
	};
	ENGINE_ASSERT_SZ(UpValue, 0x0030);
}
