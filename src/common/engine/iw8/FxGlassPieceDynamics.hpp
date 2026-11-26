#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/vec3_t.hpp"

namespace iw8 {
	class FxGlassPieceDynamics {
	public:
		int fall_time_;							// 0x0000
		std::uint32_t physics_instance_;		// 0x0004
		std::uint32_t physics_detail_instance_;	// 0x0008
		vec3_t vel_;							// 0x000C
		vec3_t avel_;							// 0x0018
		bool pending_creation_;					// 0x0024
	private:
		char pad_0025[0x0003];					// 0x0025
	};
	ENGINE_ASSERT_SZ(FxGlassPieceDynamics, 0x0028);
}
