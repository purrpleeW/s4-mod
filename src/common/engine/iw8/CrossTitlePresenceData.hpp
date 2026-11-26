#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/bdPlatformType.hpp"
#include "engine/iw8/enums/PresencePlatform.hpp"

namespace iw8 {
	#pragma pack(push,1)
	class CrossTitlePresenceData {
	public:
		std::uint64_t platform_id_;		// 0x0000
		std::uint32_t version_;			// 0x0008
		std::uint32_t title_id_;		// 0x000C
		bdPlatformType dw_platform_id_;	// 0x0010
		bool online_;					// 0x0011
	};
	#pragma pack(pop)
	ENGINE_ASSERT_SZ(CrossTitlePresenceData, 0x0012);
}
