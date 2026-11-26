#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdChannelInfo.hpp"
#include "engine/iw8/OnlineChatId.hpp"
#include "engine/iw8/enums/OnlineChatType.hpp"

namespace iw8 {
	class CachedCreateChatCallback {
	public:
		OnlineChatType chat_type_;																			// 0x0000
	private:
		char pad_0004[0x0004];																				// 0x0004
	public:
		void (*callback_)(const int, const OnlineChatId*, const OnlineChatType*, const bool, const int);	// 0x0008
		bdChannelInfo dw_channel_info_;																		// 0x0010
	private:
		char pad_0070[0x00E0];																				// 0x0070
	};
	ENGINE_ASSERT_SZ(CachedCreateChatCallback, 0x0150);
}
