#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxGlassCrackHeader.hpp"
#include "engine/iw8/FxGlassHoleHeader.hpp"
#include "engine/iw8/FxGlassVertex.hpp"

namespace iw8 {
	union FxGlassGeometryData {
		FxGlassVertex vert_;
		FxGlassHoleHeader hole_;
		FxGlassCrackHeader crack_;
		std::uint8_t as_bytes_[4];
		std::int16_t anonymous_[2];
	};
	ENGINE_ASSERT_SZ(FxGlassGeometryData, 0x0004);
}
