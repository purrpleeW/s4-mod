#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdEndpoint.hpp"

namespace iw8 {
	class bdAddrHandle : public bdReferencable {
	public:
		enum class bdAddrHandleStatus {
			BD_ADDR_NOT_CONSTRUCTED = 0,
			BD_ADDR_NOT_RESOLVED = 1,
			BD_ADDR_ERROR = 2,
			BD_ADDR_RESOLVING = 3,
			BD_ADDR_RESOLVED = 4,
			BD_ADDR_CONNECTED = 5,
			BD_ADDR_UNRESOLVED = 6
		};

		bdEndpoint endpoint_;		// 0x0010
		bdAddrHandleStatus status_;	// 0x0020
	private:
		char pad_0024[0x0004];		// 0x0024
	public:
		bdAddr real_addr_;			// 0x0028
		void* user_data_;			// 0x00C0
	};
	ENGINE_ASSERT_SZ(bdAddrHandle, 0x00C8);
}
