#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/TransientEntryBase.hpp"

namespace iw8 {
	class TransientFile : public TransientEntryBase {
	private:
		char pad_0002[0x0006];				// 0x0002
	public:
		const char* filename_;				// 0x0008
		std::uint8_t zone_flag_shifted_;	// 0x0010
	private:
		char pad_0011[0x0001];				// 0x0011
	public:
		std::uint16_t file_cost_mul_64k_;	// 0x0012
		std::uint16_t assets;				// 0x0014
		std::uint8_t type[1];				// 0x0016
		std::uint8_t state[1];				// 0x0017
	private:
		char pad_0018[0x0008];				// 0x0018
	};
}
