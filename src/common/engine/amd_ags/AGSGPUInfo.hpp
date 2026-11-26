#pragma once
#include "engine/engine_common.hpp"
#include "engine/amd_ags/AGSDeviceInfo.hpp"

namespace amd_ags {
	class AGSGPUInfo {
	public:
		const char* driver_version_;
		const char* radeon_software_version_;
		int num_devices_;
		AGSDeviceInfo* devices_;
	};
}