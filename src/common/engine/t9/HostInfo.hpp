#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/bdSecurityID.hpp"
#include "engine/t9/bdSecurityKey.hpp"
#include "engine/t9/netadr_t.hpp"
#include "engine/t9/SerializedAdr.hpp"

namespace t9 {
	/*
		netadr_t net_adr_;				// 0x0028
		SerializedAdr serialized_adr_;	// 0x0038
		bdSecurityID sec_id_;			// 0x007C
		bdSecurityKey sec_key_;			// 0x0084
		std::uint32_t server_location_;	// 0x0094
	*/
	class HostInfo {
	public:
		XUID xuid_;				// 0x0000
		char name_[32];			// 0x0008
	private:
		char pad_0028[0x00E0];	// 0x0028
	};
	ENGINE_ASSERT_SZ(HostInfo, 0x0108);
}
