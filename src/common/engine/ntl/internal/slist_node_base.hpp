#pragma once
#include "engine/engine_common.hpp"

namespace ntl::internal {
	class slist_node_base {
	public:
		slist_node_base* next_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(slist_node_base, 0x0008);
}
