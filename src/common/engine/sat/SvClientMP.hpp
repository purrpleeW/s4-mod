#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/SvClient.hpp"

namespace sat {
	class SvClientMP : public SvClient {
		// only relies on SvClient's VFT
	};
}
