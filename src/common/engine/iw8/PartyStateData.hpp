#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/msg_t.hpp"

namespace iw8 {
	class PartyStateData {
	public:
		int packet_count_;						// 0x0000
		int unfinished_part_server_times_[31];	// 0x0004
		msg_t part_msgs_[31];					// 0x0080
		std::uint8_t part_msg_bufs_[31][1252];	// 0x0748
	private:
		char pad_9EE4[0x0004];					// 0x9EE4
	};
	ENGINE_ASSERT_SZ(PartyStateData, 0x9EE8);
}
