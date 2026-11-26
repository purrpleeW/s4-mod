#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAddrHandle.hpp"
#include "engine/iw8/bdReference.hpp"

namespace iw8 {
	class NetAddress {
	public:
		int index_;							// 0x0000
	private:
		char pad_0004[0x0004];				// 0x0004
	public:
		bdReference<bdAddrHandle> handle_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(NetAddress, 0x0010);
}
