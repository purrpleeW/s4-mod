#pragma once
#include "engine/engine_common.hpp"
#include "engine/t10/SvClient.hpp"

namespace t10 {
	class SvClientMP : public SvClient {
		// only relies on SvClient's VFT
	};
}
