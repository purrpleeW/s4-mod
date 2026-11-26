#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class GSC_EXPORT_ITEM {
	public:
		std::uint32_t checksum_;		// 0x0000
		std::uint32_t address_;			// 0x0004
		std::uint32_t name_;			// 0x0008
		std::uint32_t name_space_;		// 0x000C
		std::uint32_t callback_event_;	// 0x0010
		std::uint8_t param_count_;		// 0x0014
		std::uint8_t flags_;			// 0x0015
	private:
		char pad_0016[0x0002];			// 0x0016
	};
	ENGINE_ASSERT_SZ(GSC_EXPORT_ITEM, 0x0018);
}
