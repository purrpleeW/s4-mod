#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class MovieDef {
	public:
		XHash hash_;			// 0x0000
	private:
		char pad_0008[0x0008];
	public:
		std::size_t file_size_;	// 0x0010
	private:
		char pad_0018[0x0020];	// 0x0018
	};
	ENGINE_ASSERT_SZ(MovieDef, 0x0038);
}
