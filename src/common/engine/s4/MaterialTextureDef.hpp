#pragma once
#include "engine/s4/GfxImage.hpp"

namespace s4 {
	struct MaterialTextureDef {
		std::uint8_t index_;
		std::uint8_t padding_[7];
		GfxImage* image_;
	};

	ENGINE_ASSERT_SZ(MaterialTextureDef, 0x0010);
}
