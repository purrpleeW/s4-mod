#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/bdJSONType.hpp"

namespace iw8 {
	class bdJSONDeserializer {
	public:
		bdJSONType type_;			// 0x0000
	private:
		char pad_0004[0x0004];		// 0x0004
	public:
		const char* ptr_;			// 0x0008
		const char* end_;			// 0x0010
		bool is_floating_point_;	// 0x0018
		bool parsed_;				// 0x0019
	private:
		char pad_001A[0x0002];		// 0x001A
	public:
		std::uint32_t count_;		// 0x001C
	};
	ENGINE_ASSERT_SZ(bdJSONDeserializer, 0x0020);
}
