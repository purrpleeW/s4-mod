#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	struct Image_SetupParams {
		int width_;
		int height_;
		int depth_;
		std::uint32_t num_elements_;
		std::uint32_t max_level_count_;
		std::uint32_t flags_;
		std::uint8_t format_;
		std::uint8_t padding_19_[3];
		std::uint32_t unk_1c_;
		std::uint32_t resource_flags_;
		std::uint8_t unk_24_;
		std::uint8_t padding_25_[3];
		void* custom_layout_;
		std::uint8_t unk_30_;
		std::uint8_t padding_31_[7];
	};

	ENGINE_ASSERT_SZ(Image_SetupParams, 0x0038);

	struct Image_SetupData {
		const std::uint8_t* data_[16][1536];
	};

	ENGINE_ASSERT_SZ(Image_SetupData, 0x30000);
}
