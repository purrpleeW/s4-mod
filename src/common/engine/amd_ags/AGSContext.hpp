#pragma once
#include "engine/engine_common.hpp"
#include "engine/amd_ags/AGSDeviceInfo.hpp"

namespace amd_ags {
	class AGSContext {
	public:
		IDXGIFactory1* dxgi_factory_;
		std::vector<AGSDeviceInfo> device_info_;
	};
}