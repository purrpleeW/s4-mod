#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/MRef.hpp"

namespace iw8 {
	class SBuf {
	public:
		MRef p_;	// 0x0000
		MRef e_;	// 0x0008
		MRef b_;	// 0x0010
		MRef l_;	// 0x0018
	};
	ENGINE_ASSERT_SZ(SBuf, 0x0020);
}
