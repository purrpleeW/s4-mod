#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/GenericChunkHeader.hpp"

namespace s2::hks {
	class InternString : public GenericChunkHeader {
	public:
		hksSize length_bits_;	// 0x0008
		hksUint32 hash_;		// 0x0010
		char data_[0x001E];		// 0x0014
	private:
		char pad_0032[0x0006];	// 0x0032
	};
	ENGINE_ASSERT_SZ(InternString, 0x0038);
}
