#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/KeyState.hpp"

namespace iw8 {
	class PlayerKeyState {
	private:
		char pad_0000[0x0124];	// 0x0000
	public:
		KeyState keys_[256];	// 0x0124
	private:
		char pad_0524[0x0010];	// 0x0524
	};
	ENGINE_ASSERT_SZ(PlayerKeyState, 0x0534);
}
