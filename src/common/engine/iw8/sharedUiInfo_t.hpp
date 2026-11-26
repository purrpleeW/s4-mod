#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/CachedAssets_t.hpp"
#include "engine/iw8/serverStatus_t.hpp"
#include "engine/iw8/XUID.hpp"

namespace iw8 {
	class sharedUiInfo_t {
	public:
		CachedAssets_t assets_;			// 0x00000000
		XUID party_member_xuid_;		// 0x000000A8
		serverStatus_t server_status_;	// 0x000000B0
	};
	ENGINE_ASSERT_SZ(sharedUiInfo_t, 0x000141C8);
}
