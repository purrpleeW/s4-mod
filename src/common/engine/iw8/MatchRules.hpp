#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class MatchRules {
	public:
  		std::uint8_t data_[4097];
  		std::uint32_t checksum_;
	};
	ENGINE_ASSERT_SZ(MatchRules, 0x1008);
}