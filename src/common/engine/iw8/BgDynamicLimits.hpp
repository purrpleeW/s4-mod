#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class BgDynamicLimits {
	public:
		class BgDynamicLimitsData {
		public:
			std::uint32_t effect_bits_;						// 0x0000
			std::uint32_t model_bits_;						// 0x0004
			std::uint32_t es_index_bits_;					// 0x0008
			std::uint32_t client_tag_bits_;					// 0x000C
			int effect_names_count_;						// 0x0010
			std::uint32_t case_sensitive_config_strings_;	// 0x0014
			std::uint32_t max_config_strings_;				// 0x0018
			std::uint32_t scriptable_instance_bits_;		// 0x001C
			std::uint32_t client_fire_event_param_bits_;	// 0x0020
		};
		ENGINE_ASSERT_SZ(BgDynamicLimitsData, 0x0024);
	};
	ENGINE_ASSERT_SZ(BgDynamicLimits, 0x0001);
}
