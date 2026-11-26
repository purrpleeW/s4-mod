#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxSpatialFrame.hpp"
#include "engine/iw8/vec2_t.hpp"

namespace iw8 {
	class FxGlassInitPieceState {
	public:
		FxSpatialFrame frame_;			// 0x0000
		float radius_;					// 0x001C
		vec2_t tex_coord_origin_;		// 0x0020
		std::uint32_t support_mask_;	// 0x0028
		float area_x2_;					// 0x002C
		std::uint8_t def_index_;		// 0x0030
		std::uint8_t vert_count_;		// 0x0031
		std::uint8_t fan_data_count_;	// 0x0032
	private:
		char pad_0033[0x0001];			// 0x0033
	};
	ENGINE_ASSERT_SZ(FxGlassInitPieceState, 0x0034);
}
