#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/KeyState.hpp"

namespace t9 {
	class PlayerKeyState {
	public:
		bool overstrike_mode_;	// 0x0000
	private:
		char pad_0001[0x0003];	// 0x0001
	public:
		int any_key_down_;		// 0x0004
		KeyState keys_[256];	// 0x0008
	};
	ENGINE_ASSERT_SZ(PlayerKeyState, 0x1008);
}
