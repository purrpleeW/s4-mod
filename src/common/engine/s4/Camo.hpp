#pragma once
#include "engine/s4/MaterialTextureDef.hpp"
#include "engine/s4/enums/scr_string_t.hpp"
#include "engine/s4/vec4_t.hpp"

#include <cstddef>

namespace s4 {
	struct Camo {
		const char* name_;			 			 // 0x00
		scr_string_t internal_name_; 			 // 0x08
		std::uint32_t padding_0c_;
		MaterialTextureDef* texture_defs_;		 // 0x10
		std::uint32_t texture_def_count_;		 // 0x18
		std::uint32_t color_tint_[3];			 // 0x1C
		std::uint32_t albedo_map_scale_bias_[3]; // 0x28
		std::uint32_t normal_map_scale_bias_[3]; // 0x34
		std::uint32_t color_texture_ids_[3];	 // 0x40, slots 1-3
		std::uint32_t normal_texture_ids_[3];	 // 0x4C, slots 9-11
		std::uint32_t gray_texture_ids_[3];		 // 0x58, slots 25-27
		vec4_t uv_scale_offset_[3];	 			 // 0x64
		float uv_rotation_angle_[3]; 			 // 0x94 (animation input)
		vec4_t uv_rotate_[3];		 			 // 0xA0
		std::uint8_t unk_d0_[0x4E];
		std::uint8_t blend_map_channels_;
		std::uint8_t unk_11f_[0x29];
	};

	ENGINE_ASSERT_SZ(Camo, 0x0148);
	static_assert(offsetof(Camo, blend_map_channels_) == 0x11E);
}
