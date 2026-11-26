#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/vec2_t.hpp"

namespace iw8 {
	class FxGlassPieceState {
	public:
		vec2_t tex_coord_origin_;		// 0x0000
		std::uint32_t support_mask_;	// 0x0008
		std::uint32_t geo_data_start_;	// 0x000C
		std::uint16_t init_index_;		// 0x0010
		std::uint8_t def_index_;		// 0x0012
	private:
		char pad_0013[0x0001];			// 0x0013
	public:
		std::uint8_t vert_count_;		// 0x0014
		std::uint8_t hole_data_count_;	// 0x0015
		std::uint8_t crack_data_count_;	// 0x0016
		std::uint8_t fan_data_count_;	// 0x0017
		std::uint16_t flags_;			// 0x0018
	private:
		char pad_001A[0x0002];			// 0x001A
	public:
		float area_x2_;					// 0x001C
	};
	ENGINE_ASSERT_SZ(FxGlassPieceState, 0x0020);
}
