#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"
#include "engine/iw8/MRef.hpp"

namespace iw8 {
	class GCproto {
	public:
		GCRef next_gc_;				// 0x0000
		std::uint8_t marked_;		// 0x0008
		std::uint8_t gct_;			// 0x0009
		std::uint8_t num_params_;	// 0x000A
		std::uint8_t frame_size_;	// 0x000B
		std::uint32_t size_bc_;		// 0x000C
		std::uint32_t unused_gc64_;	// 0x0010
	private:
		char pad_0014[0x0004];		// 0x0014
	public:
		GCRef gc_list_;				// 0x0018
		MRef k_;					// 0x0020
		MRef uv_;					// 0x0028
		std::uint32_t size_kgc_;	// 0x0030
		std::uint32_t size_kn_;		// 0x0034
		std::uint32_t size_pt_;		// 0x0038
		std::uint8_t size_uv_;		// 0x003C
		std::uint8_t flags_;		// 0x003D
		std::uint16_t trace_;		// 0x003E
		GCRef chunk_name_;			// 0x0040
		int first_line_;			// 0x0048
		int num_line_;				// 0x004C
		MRef line_info_;			// 0x0050
		MRef uv_info_;				// 0x0058
		MRef var_info_;				// 0x0060
	};
	ENGINE_ASSERT_SZ(GCproto, 0x0068);
}
