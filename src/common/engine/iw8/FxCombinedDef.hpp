#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class FxCombinedDef {
	public:
		const ParticleSystemDef* particle_system_def_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(FxCombinedDef, 0x0008);
}
