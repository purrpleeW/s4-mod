#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class DBCreateDefaultOptions {
	public:
		bool create_default_;		// 0x0000
		bool copy_default_strings_;	// 0x0001
	};
	ENGINE_ASSERT_SZ(DBCreateDefaultOptions, 0x0002);
}
