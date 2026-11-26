#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxCombinedDef.hpp"
#include "engine/iw8/MaterialTextureDef.hpp"
#include "engine/iw8/vec4_t.hpp"
#include "engine/iw8/enums/scr_string_t.hpp"

namespace iw8 {
	class Camo {
	public:
		const char* name_;							// 0x0000
		scr_string_t internal_name_;				// 0x0008
	private:
		char pad_000C[0x0004];						// 0x000C
	public:
		MaterialTextureDef* texture_defs_;			// 0x0010
		std::uint32_t texture_def_count_;			// 0x0018
		std::uint32_t color_tint_[3];				// 0x001C
		std::uint32_t albedo_map_scale_bias_[3];	// 0x0028
		std::uint32_t normal_map_scale_bias_[3];	// 0x0034
		vec4_t uv_scale_offset_[3];					// 0x0040
		vec4_t uv_rotate_[3];						// 0x0070

		union {
			class v1 {
			public:
				std::uint8_t blend_map_channels_;	// 0x00A0
			private:
				char pad_00A1[0x0007];				// 0x00A1
			public:
				FxCombinedDef veh_vfx_tail_light_;	// 0x00A8
				FxCombinedDef veh_vfx_door_smoke_;	// 0x00B0
			} v1_;

			class v2 {
			private:
				char pad_00A0[0x005A];              // 0x00A0
			public:
				std::uint8_t blend_map_channels_;   // 0x00FA
			private:
				char pad_00FB[0x002D];              // 0x00FB
			} v2_;
		};

		std::uint8_t& get_blend_map_channels();
	};
}
