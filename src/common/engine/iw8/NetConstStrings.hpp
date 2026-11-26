#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/NetConstStringSource.hpp"
#include "engine/iw8/enums/NetConstStringType.hpp"

namespace iw8 {
	class NetConstStrings {
	public:
		const char* name_;					// 0x0000
		NetConstStringType string_type_;	// 0x0008
		NetConstStringSource source_type_;	// 0x000C
		std::uint32_t flags_;				// 0x0010
		std::uint32_t entry_count_;			// 0x0014
		const char** string_list_;			// 0x0018
	};
	ENGINE_ASSERT_SZ(NetConstStrings, 0x0020);
}
