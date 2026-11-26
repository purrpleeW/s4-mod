#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/GfxImageFlags.hpp"
#include "engine/iw8/enums/GfxPixelFormat.hpp"

namespace iw8 {
	class Image_SetupParams {
	public:
		using CustomAllocCallback = void*(std::size_t, std::size_t, void*);

		int width_;										// 0x0000
		int height_;									// 0x0004
		int depth_;										// 0x0008
		std::uint32_t num_elements_;					// 0x000C
		std::uint32_t max_level_count_;					// 0x0010
		GfxImageFlags flags_;							// 0x0014
		GfxPixelFormat format_;							// 0x0018
	private:
		char pad_001C[0x0004];							// 0x001C
	public:
		CustomAllocCallback* custom_alloc_func_;		// 0x0020
		void* custom_alloc_user_data_;					// 0x0028
		const XG_RESOURCE_LAYOUT* custom_layout_;		// 0x0030
		D3D12_TEXTURE_LAYOUT texture_layout_override_;	// 0x0038
	private:
		char pad_003C[0x0004];							// 0x003C
	};
	ENGINE_ASSERT_SZ(Image_SetupParams, 0x0040);
}
