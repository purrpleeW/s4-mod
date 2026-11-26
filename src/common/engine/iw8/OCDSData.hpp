#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/FenceState.hpp"
#include "engine/iw8/enums/OnlinePlayFailureReason.hpp"

namespace iw8 {
	class OCDSData {
	public:
		FenceState state_;
  		int start_time_;
  		OnlinePlayFailureReason error_code_;
	};
	ENGINE_ASSERT_SZ(OCDSData, 0x000C);
}
