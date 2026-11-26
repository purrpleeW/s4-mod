#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/EntityClass.hpp"

namespace iw8 {
	class scr_entref_t {
	public:
		std::uint32_t ent_num_;	// 0x0000
		EntityClass ent_class_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(scr_entref_t, 0x0008);
}
