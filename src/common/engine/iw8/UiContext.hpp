#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/LocalClientNum_t.hpp"

namespace iw8 {
	class UiContext {
	public:
		LocalClientNum_t local_client_num_;	// 0x0000
		float bias_;						// 0x0004
		int real_time_;						// 0x0008
		class _cursor {
		public:
			float x_;						// 0x0000
			float y_;						// 0x0004
		} cursor;							// 0x000C
		int screen_width_;					// 0x0014
		int screen_height_;					// 0x0018
		float screen_aspect_;				// 0x001C
		float blur_radius_out_;				// 0x0020
		int subtitle_id_;					// 0x0024
	};
	ENGINE_ASSERT_SZ(UiContext, 0x0028);
}
