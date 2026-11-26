#include "engine/engine_common.hpp"
#include "engine/iw8/vec3_t.hpp"

namespace iw8 {
	class RefdefViewOrg {
	public:
		vec3_t org_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(RefdefViewOrg, 0x000C);
}
