#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/HksValue.hpp"

namespace s2 {
	class HksObject {
	public:
		hksUint32 t;			// 0x0000
	private:
		char pad_0004[0x0004];	// 0x0004
	public:
		HksValue v;				// 0x0008
	};
	ENGINE_ASSERT_SZ(HksObject, 0x0010);

	using HksRegister = HksObject;
}
