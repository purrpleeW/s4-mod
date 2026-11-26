#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxSpatialFrame.hpp"

namespace iw8 {
	union FxGlassPiecePlace {
		class {
		public:
			FxSpatialFrame frame;	// 0x0000
			float radius;			// 0x001C
		};
		std::uint32_t next_free_;
	};
	ENGINE_ASSERT_SZ(FxGlassPiecePlace, 0x0020);
}
