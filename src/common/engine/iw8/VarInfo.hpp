#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"

namespace iw8 {
	class VarInfo {
	public:
		GCRef name_;			// 0x0000
		BCPos start_pc_;		// 0x0008
		BCPos end_pc_;			// 0x000C
		std::uint8_t slot_;		// 0x0010
		std::uint8_t info_;		// 0x0011
	private:
		char pad_0012[0x0006];	// 0x0012
	};
	ENGINE_ASSERT_SZ(VarInfo, 0x0018);
}
