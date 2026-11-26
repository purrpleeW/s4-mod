#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class ClientCustomizationInfo {
	public:
		std::uint32_t model_index_[3];		// 0x0000
		std::uint32_t model_index_mlg_[3];	// 0x000C
	};
	ENGINE_ASSERT_SZ(ClientCustomizationInfo, 0x0018);
}
