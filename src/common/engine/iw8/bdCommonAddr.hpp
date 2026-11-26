#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAddr.hpp"
#include "engine/iw8/bdFixedArray.hpp"
#include "engine/iw8/bdReferencable.hpp"
#include "engine/iw8/bdRelayJoinData.hpp"
#include "engine/iw8/enums/bdNATType.hpp"
#include "engine/iw8/enums/bdPlatformID.hpp"

namespace iw8 {
	class bdCommonAddr : public bdReferencable {
	public:
		bdFixedArray<bdAddr, 5> local_addrs_;	// 0x0010
		bdAddr public_addr_;					// 0x0310
		bdNATType nat_type_;					// 0x03A8
		std::uint32_t hash_;					// 0x03AC
		bool is_loopback_;						// 0x03B0
	private:
		char pad_03B1[0x0003];					// 0x03B1
	public:
		bdPlatformID platform_id_;				// 0x03B4
		bdRelayJoinData join_data_;				// 0x03B8
	};
	ENGINE_ASSERT_SZ(bdCommonAddr, 0x04E0);
}
