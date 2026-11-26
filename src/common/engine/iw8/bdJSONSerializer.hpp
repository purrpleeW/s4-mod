#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/bdJSONType.hpp"

namespace iw8 {
	class bdJSONSerializer {
	public:
		class bdJSONTypeFirst {
		public:
			bdJSONType type_;			// 0x0000
			bool first_;				// 0x0004
		private:
			char pad_0005[0x0003];		// 0x0005
		};

		char* buffer_;					// 0x0000
		std::uint32_t buffer_size_;		// 0x0008
		std::uint32_t buffer_index_;	// 0x000C
		bdJSONTypeFirst stack_[0x20];	// 0x0010
		std::uint32_t stack_depth_;		// 0x0110
	private:
		char pad_0114[0x0004];			// 0x0114
	};
	ENGINE_ASSERT_SZ(bdJSONSerializer, 0x0118);
}
