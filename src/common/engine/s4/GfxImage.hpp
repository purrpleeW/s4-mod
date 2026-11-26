#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class GfxImage {
	public:
		const char* name_;				  // 0x00
		std::uint8_t* packed_atlas_data_; // 0x08
		std::uint32_t texture_id_;		  // 0x10
		std::uint32_t flags_;			  // 0x14
		std::uint32_t total_size_;		  // 0x18
		std::uint32_t semantic_specific_; // 0x1C
		std::uint16_t width_;			  // 0x20
		std::uint16_t height_;			  // 0x22
		std::uint16_t depth_;			  // 0x24
		std::uint16_t num_elements_;	  // 0x26
		std::uint16_t atlas_info_;		  // 0x28
		std::uint8_t format_;			  // 0x2A
		std::uint8_t semantic_category_;  // 0x2B
		std::uint8_t level_count_;		  // 0x2C
		std::uint8_t unk_2d_;
		std::uint8_t streamed_part_count_; // 0x2E
		std::uint8_t picmip_;			   // 0x2F
		std::uint64_t unk_30_;
		void* streams_;				  	  // 0x38
		void* primed_image_;		      // 0x40
		std::uint8_t* resident_data_;     // 0x48
	};

	ENGINE_ASSERT_SZ(GfxImage, 0x0050);
}
