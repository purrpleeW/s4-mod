#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/PresencePlatform.hpp"

namespace iw8 {
	class PresencePlatformMapping {
	public:
		PresencePlatform platform_id_;          // 0x0000
		const char *platform_identifier_;       // 0x0008
		const char *presence_platform_name_;    // 0x0010
	};
	ENGINE_ASSERT_SZ(PresencePlatformMapping, 0x0018);
}