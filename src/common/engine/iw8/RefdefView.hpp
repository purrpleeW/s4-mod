#include "engine/engine_common.hpp"
#include "engine/iw8/GfxFovVector.hpp"
#include "engine/iw8/RefdefViewOrg.hpp"
#include "engine/iw8/tmat33_t.hpp"

namespace iw8 {
	class RefdefView {
	public:
		GfxFovVector fov_;							// 0x0000
		RefdefViewOrg org_;							// 0x0008
		tmat33_t<vec3_t> axis_;						// 0x0014
		float z_near_;								// 0x0038
		GfxFovVector depth_hack_fov_;				// 0x003C
		float focal_length_;						// 0x0044
		GfxFovVector default_fov_;					// 0x0048
		vec3_t visibility_offset_;					// 0x0050
		float visibility_query_distance_;			// 0x005C
		bool visibility_offset_apply_to_ref_point_;	// 0x0060
	private:
		char pad_0061[0x0003];						// 0x0061
	public:
		std::uint32_t refdef_view_org_aab_;			// 0x0064
		std::uint32_t refdef_view_org_set_aab_;		// 0x0068
	};
}
