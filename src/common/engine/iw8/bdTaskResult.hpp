#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdByteBuffer.hpp"
#include "engine/iw8/bdReference.hpp"

namespace iw8 {
	class bdTaskResult {
	public:
		virtual bool deserialize(bdReference<bdByteBuffer>) { return false; }
		virtual std::uint32_t sizeOf() { return 0; }
	private:
		char pad_0008[0x0018];	// 0x0008
	};
	ENGINE_ASSERT_SZ(bdTaskResult, 0x0020);
}
