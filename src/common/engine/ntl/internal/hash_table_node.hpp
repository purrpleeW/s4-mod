#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/intrusive_slist_node.hpp"
#include "engine/ntl/pair.hpp"

namespace ntl::internal {
	template <typename A, typename B>
	class hash_table_node : public intrusive_slist_node, public pair<A, B> {
		// no members
	};
}
