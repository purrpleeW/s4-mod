#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxGlassGeometryData.hpp"
#include "engine/iw8/FxGlassInitPieceState.hpp"
#include "engine/iw8/FxGlassPieceDynamics.hpp"
#include "engine/iw8/FxGlassPiecePlace.hpp"
#include "engine/iw8/FxGlassPieceState.hpp"

namespace iw8 {
	class FxGlassSystem {
	public:
		int time_;									// 0x0000
		int prev_time_;								// 0x0004
		std::uint32_t piece_limit_;					// 0x0008
		std::uint32_t piece_word_count_;			// 0x000C
		std::uint32_t cell_count_;					// 0x0010
		std::uint32_t active_piece_count_;			// 0x0014
		std::uint32_t first_free_piece_;			// 0x0018
		std::uint32_t geo_data_limit_;				// 0x001C
		std::uint32_t geo_data_count_;				// 0x0020
		std::uint32_t init_geo_data_count_;			// 0x0024
		FxGlassPiecePlace* piece_places_;			// 0x0028
		FxGlassPieceState* piece_states_;			// 0x0030
		FxGlassPieceDynamics* piece_dynamics_;		// 0x0038
		FxGlassGeometryData* geo_data_;				// 0x0040
		std::uint32_t* is_in_use_;					// 0x0048
		std::uint32_t* cell_bits_;					// 0x0050
		std::uint8_t* vis_data_;					// 0x0058
		vec3_t* link_org_;							// 0x0060
		float* half_thickness_;						// 0x0068
		FxGlassGeometryData* init_geo_data_;		// 0x0070
		bool need_to_compact_data_;					// 0x0078
		std::uint8_t init_count_;					// 0x0079
	private:
		char pad_007A[0x0002];						// 0x007A
	public:
		float effect_chance_accum_;					// 0x007C
		int last_piece_deletion_time;				// 0x0080
		std::uint32_t init_piece_count_;			// 0x0084
		FxGlassInitPieceState* init_piece_states_;	// 0x0088
	};
	ENGINE_ASSERT_SZ(FxGlassSystem, 0x0090);
}
