#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/PacketMode.hpp"
#include "engine/t9/enums/netsrc_t.hpp"

namespace t9 {
	class msg_t {
	public:
		bool overflowed_;				// 0x0000
		bool read_only_;				// 0x0001
	private:
		char pad_0002[0x0006];			// 0x0002
	public:
		std::uint8_t* data_;			// 0x0008
		std::uint8_t* split_data_;		// 0x0010
		int max_size_;					// 0x0018
		int cur_size_;					// 0x001C
		int split_size_;				// 0x0020
		int read_count_;				// 0x0024
		int bit_;						// 0x0028
		int last_entity_ref_;			// 0x002C
		bool flush_;					// 0x0030
	private:
		char pad_0031[0x0003];			// 0x0031
	public:
		netsrc_t target_local_net_id_;	// 0x0034
		PacketMode analysis_;			// 0x0038
	};
	ENGINE_ASSERT_SZ(msg_t, 0x0040);
}
