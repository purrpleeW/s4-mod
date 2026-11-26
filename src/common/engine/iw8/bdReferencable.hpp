#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAtomic.hpp"

namespace iw8 {
	class bdReferencable {
	public:
		virtual ~bdReferencable() = 0;

		bdAtomic<int> ref_count_;	// 0x0008
	private:
		char pad_000C[0x0004];		// 0x000C
	};
	ENGINE_ASSERT_SZ(bdReferencable, 0x0010);
}
