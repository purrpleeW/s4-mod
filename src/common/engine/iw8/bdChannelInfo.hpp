#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdTaskResult.hpp"

namespace iw8 {
	class bdChannelInfo : public bdTaskResult {
	public:
		bool deserializeWithCategory(bdReference<bdByteBuffer>) { return false; }
	private:
		char pad_0020[0x0010];				// 0x0020
	public:
		std::uint64_t channel_id_;			// 0x0030
		std::uint32_t num_subscribers_;		// 0x0038
		std::uint32_t max_subscribers_;		// 0x003C
		std::uint64_t admin_id_;			// 0x0040
		bool is_password_required_;			// 0x0048
		bool is_public_channel_;			// 0x0049
	private:
		char pad_004A[0x0002];				// 0x004A
	public:
		std::uint32_t data_version_;		// 0x004C
	private:
		char pad_0050[0x0010];				// 0x0050
	};
	ENGINE_ASSERT_SZ(bdChannelInfo, 0x0060);
}
