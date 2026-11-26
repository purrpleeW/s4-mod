#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class TransientEntryBase {
	public:
		std::uint8_t in_use_ : 1;					// 0x0000.0
		std::uint8_t is_overridden_ : 1;			// 0x0000.1
		std::uint8_t override_needs_reload_ : 1;	// 0x0000.2
		std::uint8_t override_needs_unload_ : 1;	// 0x0000.3
		std::uint8_t reference_count_;				// 0x0001
	};
	ENGINE_ASSERT_SZ(TransientEntryBase, 0x0002);
}
