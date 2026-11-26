#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/slist_node_base.hpp"

namespace ntl::internal {
	class slist_head_base {
	public:
		slist_node_base sentinel_;
	};
}
