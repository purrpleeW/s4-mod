#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAddr.hpp"

namespace iw8 {
	class bdRelayJoinData {
	public:
		std::uint8_t encrypted_[86];	// 0x0000
		std::uint8_t version_;			// 0x0056
		std::uint8_t secret_[16];		// 0x0057
	private:
		char pad_0067[0x0001];			// 0x0067
	public:
		std::uint64_t relay_id_;		// 0x0068
		std::uint32_t routing_id_;		// 0x0070
	private:
		char pad_0074[0x0004];			// 0x0074
	public:
		std::uint64_t join_id_;			// 0x0078
		bdAddr relay_addr_;				// 0x0080
		std::uint8_t verification_[16];	// 0x0118
	};
	ENGINE_ASSERT_SZ(bdRelayJoinData, 0x0128);
}
