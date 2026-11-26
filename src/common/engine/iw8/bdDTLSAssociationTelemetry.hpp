#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdAddr.hpp"
#include "engine/iw8/bdStopwatch.hpp"

namespace iw8 {
	class bdDTLSAssociationTelemetry {
	public:
		int last_state_;									// 0x0000
		std::uint32_t lifetime_ms_;							// 0x0004
		std::uint32_t stage_init_ms_since_start_;			// 0x0008
		std::uint32_t stage_init_ack_ms_since_start_;		// 0x000C
		std::uint32_t stage_cookie_echo_ms_since_start_;	// 0x0010
		std::uint32_t stage_cookie_ack_ms_since_start_;		// 0x0014
		std::uint32_t established_ms_since_start_;			// 0x0018
	private:
		char pad_001C[0x0004];								// 0x001C
	public:
		bdAddr peer_addr_;									// 0x0020
		std::uint8_t init_resends_;							// 0x00B8
		std::uint8_t cookie_resends_;						// 0x00B9
	private:
		char pad_00BA[0x0002];								// 0x00BA
	public:
		int send_seq_num_;									// 0x00BC
		int recv_seq_num_;									// 0x00C0
		bool telemetry_pending_;							// 0x00C4
	private:
		char pad_00C5[0x0003];								// 0x00C5
	public:
		bdStopwatch age_;									// 0x00C8
	};
	ENGINE_ASSERT_SZ(bdDTLSAssociationTelemetry, 0x00D0);
}
