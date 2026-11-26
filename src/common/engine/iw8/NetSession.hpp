#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XSECURITY_INFO.hpp"

namespace iw8 {
	class NetSession {
	public:
		XSECURITY_INFO info_;			// 0x0000
		int reference_count_;			// 0x0018
	};
	ENGINE_ASSERT_SZ(NetSession, 0x001C);
}
