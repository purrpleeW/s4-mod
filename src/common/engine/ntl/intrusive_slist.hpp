#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/slist_head_base.hpp"

namespace ntl {
	class intrusive_slist {
	public:
		internal::slist_head_base list_head_;
	};
	ENGINE_ASSERT_SZ(intrusive_slist, 0x0008);
}
