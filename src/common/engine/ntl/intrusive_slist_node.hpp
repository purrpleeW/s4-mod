#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/slist_node_base.hpp"

namespace ntl {
	class intrusive_slist_node : public internal::slist_node_base {
		// no members
	};
	ENGINE_ASSERT_SZ(intrusive_slist_node, 0x0008);
}
