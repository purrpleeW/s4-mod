#pragma once
#include "engine/engine_common.hpp"
#include "engine/amd_ags/AGSRect.hpp"

namespace amd_ags {
	class AGSDisplayInfo {
	public:
		char name_[256];
		char display_device_name_[32];
		std::uint32_t is_primary_display_ : 1;
		std::uint32_t hdr10_ : 1;
		std::uint32_t dolby_vision_ : 1;
		std::uint32_t freesync_ : 1;
		std::uint32_t freesync_hdr_ : 1;
		std::uint32_t eyefinity_in_group_ : 1;
		std::uint32_t eyefinity_preferred_display_ : 1;
		std::uint32_t eyefinity_in_portrait_mode_ : 1;
		std::uint32_t reserved_padding_ : 24;
		int max_resolution_x_;
		int max_resolution_y_;
		float max_refresh_rate_;
		AGSRect current_resolution_;
		AGSRect visible_resolution_;
		float current_refresh_rate_;
		int eyefinity_grid_coord_x_;
		int eyefinity_grid_coord_y_;
		double chromaticity_red_x_;
		double chromaticity_red_y_;
		double chromaticity_green_x_;
		double chromaticity_green_y_;
		double chromaticity_blue_x_;
		double chromaticity_blue_y_;
		double chromaticity_white_point_x_;
		double chromaticity_white_point_y_;
		double screen_diffuse_reflectance_;
		double screen_specular_reflectance_;
		double min_luminance_;
		double max_luminance_;
		double avg_luminance_;
		int logical_display_index_;
		int adl_adapter_index_;
		int reserved_;
	};
}