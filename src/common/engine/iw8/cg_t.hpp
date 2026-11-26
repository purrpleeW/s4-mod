#include "engine/engine_common.hpp"
#include "engine/iw8/RefdefView.hpp"
#include "engine/iw8/vec3_t.hpp"

namespace iw8 {
	class cg_t {
	public:
		int& get_frametime() {
			//return DEREF_PTR_AS(int, PTR_AS(std::uintptr_t, this) + 0x659C);
			return DEREF_PTR_AS(int, PTR_AS(std::uintptr_t, this) + 0x27CB4);
		}

		RefdefView& get_refdef_view() {
			//return DEREF_PTR_AS(RefdefView, PTR_AS(std::uintptr_t, this) + 0x6810);
			return DEREF_PTR_AS(RefdefView, PTR_AS(std::uintptr_t, this) + 0x2700);
		}

		vec3_t& get_refdef_view_angles() {
			//return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x74A0);
			return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x60574);
		}

		vec3_t& get_target_camera_offset() {
			//return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x7AE0);
			return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x5E554);
		}

		vec3_t& get_current_camera_offset() {
			//return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x7AEC);
			return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x45F78);
		}

		vec3_t& get_target_camera_angle_offset() {
			//return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x7AF8);
			return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x45FB0);
		}

		vec3_t& get_current_camera_angle_offset() {
			//return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x7B04);
			return DEREF_PTR_AS(vec3_t, PTR_AS(std::uintptr_t, this) + 0x24650);
		}
	};
	ENGINE_ASSERT_SZ(cg_t, 0x0001);
}
