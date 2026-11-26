#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/OnlineChatMessageProfanityFilterState.hpp"

namespace iw8 {
	union OnlineChatMessage {
		using State = int;

		class {
		public:
			std::uint64_t from_id_;											// 0x0000
			std::uint64_t to_id_;											// 0x0008
			char from_gamertag_[36];										// 0x0010
			char message_[500];												// 0x0034
			std::uint32_t received_time_;									// 0x0228
			std::uint8_t state_[4];											// 0x022C
			OnlineChat* chat_;												// 0x0230
			OnlineChatMessageProfanityFilterState profanity_filter_state_;	// 0x0238
		} v1_;

		class {
		public:
			char from_gamertag_[36];										// 0x0000
			char message_[500];												// 0x0024
			OnlineChat* chat_;												// 0x0218
			std::uint64_t from_id_;											// 0x0220
			std::uint64_t to_id_;											// 0x0228
			std::uint32_t received_time_;									// 0x0230
			std::uint8_t state_[4];											// 0x0234
		} v2_;

		char* get_from_gamertag();
		std::uint64_t& get_from_id();
		char* get_message();
		std::uint32_t& get_received_time();
		std::uint8_t* get_state();
		std::uint64_t& get_to_id();
	};
}
