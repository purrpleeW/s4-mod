#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XAssetHeader.hpp"

namespace iw8 {
#	pragma pack(push, 1)
	class DB_AssetEntry {
	public:
		XAssetHeader header_;			// 0x0000
		std::uint32_t next_hash_;		// 0x0008
		std::uint32_t next_stashed_;	// 0x000C
		volatile std::uint8_t in_use_;	// 0x0010
		std::uint8_t type_;				// 0x0011
		std::uint16_t zone_index_;		// 0x0012
	};
#	pragma pack(pop)
	ENGINE_ASSERT_SZ(DB_AssetEntry, 0x0014);
}
