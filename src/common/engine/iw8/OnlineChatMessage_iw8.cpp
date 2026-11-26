#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw8/OnlineChatMessage.hpp"
#include "identification/game.hpp"

namespace iw8 {
	bool ocm_use_v2() {
		static bool use_v2 = identification::game::is_greater_or_eq("1.46.0");
		return use_v2;
	}

	char* OnlineChatMessage::get_from_gamertag() {
		if (ocm_use_v2()) {
			return this->v2_.from_gamertag_;
		}
		return this->v1_.from_gamertag_;
	}

	std::uint64_t& OnlineChatMessage::get_from_id() {
		if (ocm_use_v2()) {
			return this->v2_.from_id_;
		}
		return this->v1_.from_id_;
	}

	char* OnlineChatMessage::get_message() {
		if (ocm_use_v2()) {
			return this->v2_.message_;
		}
		return this->v1_.message_;
	}

	std::uint32_t& OnlineChatMessage::get_received_time() {
		if (ocm_use_v2()) {
			return this->v2_.received_time_;
		}
		return this->v1_.received_time_;
	}

	std::uint8_t* OnlineChatMessage::get_state() {
		if (ocm_use_v2()) {
			return this->v2_.state_;
		}
		return this->v1_.state_;
	}

	std::uint64_t& OnlineChatMessage::get_to_id() {
		if (ocm_use_v2()) {
			return this->v2_.to_id_;
		}
		return this->v1_.to_id_;
	}
}
#endif
