#pragma once
#include "engine/engine_common.hpp"
#include "engine/amd_ags/AGSDisplayInfo.hpp"
#include "engine/amd_ags/enums/AGSAsicFamily.hpp"

namespace amd_ags {
	class AGSDeviceInfo {
	public:
		const char* adapter_string_;
		AGSAsicFamily asic_family_;
		std::uint32_t is_apu_ : 1;
		std::uint32_t is_primary_device_ : 1;
		std::uint32_t is_external_ : 1;
		std::uint32_t reserved_padding_ : 29;
		int vendor_id_;
		int device_id_;
		int revision_id_;
		int num_cus_;
		int num_wgps_;
		int num_rops_;
		int core_clock_;
		int memory_clock_;
		int memory_bandwidth_;
		float tera_flops_;
		std::uint64_t local_memory_in_bytes_;
		std::uint64_t shared_memory_in_bytes_;
		int num_displays_;
		AGSDisplayInfo* displays_;
		int eyefinity_enabled_;
		int eyefinity_grid_width_;
		int eyefinity_grid_height_;
		int eyefinity_resolution_x_;
		int eyefinity_resolution_y_;
		int eyefinity_bezel_compensated_;
		int adl_adapter_index_;
		int reserved_;
	};
}