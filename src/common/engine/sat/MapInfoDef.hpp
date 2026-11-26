#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class MapInfoDef {
	public:
		XHash hash_;				// 0x0000
		const char* map_;			// 0x0008
		const char* gametype_;		// 0x0010
		const char* script_parent_;	// 0x0018
		const char* str3_;			// 0x0020
		const char* base_line_;		// 0x0028
		const char* map_image_;		// 0x0030
	private:
		char pad_0038[0x0008];		// 0x0038
	public:
		const char* omnvars_map_;	// 0x0040
		const char* str7_;			// 0x0048
	private:
		char pad_0050[0x0098];		// 0x0050
	};
	ENGINE_ASSERT_SZ(MapInfoDef, 0x00E8);
}
