#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/netipv4_t.hpp"
#include "engine/t9/enums/netadrtype_t.hpp"
#include "engine/t9/enums/netsrc_t.hpp"

namespace t9 {
	class netadr_t {
	public:
		netipv4_t ip_v4_;		// 0x0000
		std::uint16_t port_;	// 0x0004
	private:
		char pad_0006[0x0002];	// 0x0006
	public:
		netadrtype_t type_;		// 0x0008
		netsrc_t local_net_id_;	// 0x000C
	};
	ENGINE_ASSERT_SZ(netadr_t, 0x0010);
}
