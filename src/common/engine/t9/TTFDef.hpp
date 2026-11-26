#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/TTFKerningEntry.hpp"

namespace t9 {
	class TTFDef {
	public:
		std::int64_t name_hash_;					// 0x00000000
		int file_len_;								// 0x00000008
	private:
		char pad_0000000C[0x00000004];				// 0x0000000C
	public:
		const char* file_;							// 0x00000010
		void* ft_face_;								// 0x00000018
		TTFKerningEntry kerning_cache_[0x4000];		// 0x00000020
	};
	ENGINE_ASSERT_SZ(TTFDef, 0x00020020);
}
