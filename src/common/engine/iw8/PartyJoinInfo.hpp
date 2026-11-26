#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/NetConnection.hpp"
#include "engine/iw8/PartyData.hpp"
#include "engine/iw8/PartyJoinChallenge.hpp"
#include "engine/iw8/PartyMemberJoinInfo.hpp"
#include "engine/iw8/PartyStateData.hpp"
#include "engine/iw8/enums/PartyJoinResponse.hpp"
#include "engine/iw8/enums/PartyJoinState.hpp"
#include "engine/iw8/enums/PartyJoinType.hpp"
#include "engine/iw8/enums/PartyType.hpp"

namespace iw8 {
	class PartyJoinInfo {
	public:
		PartyJoinState state_;						// 0x0000
		PartyJoinType join_type_;					// 0x0004
		PartyType party_type_;						// 0x0008
		bool joining_a_game_in_progress_;			// 0x000C
	private:
		char pad_000D[0x0005];						// 0x000D
	public:
		SessionData* session_;						// 0x0018
		NetConnection connections_[1];				// 0x0020
		PartyData* atomic_party_;					// 0x0040
		int next_broadcast_time_;					// 0x0048
	private:
		char pad_004C[0x0004];						// 0x004C
	public:
		PartyMemberJoinInfo member_join_info_[200];	// 0x0050
		PartyJoinResponse host_response_[2];		// 0x1950
		std::uint8_t member_count_;					// 0x1958
		std::uint8_t sub_party_count_;				// 0x1959
		std::uint8_t sub_party_list_[200];			// 0x195A
		std::uint8_t party_cod_play_mode_[1];		// 0x1A22
	private:
		char pad_1A23[0x0001];						// 0x1A23
	public:
		int join_accepted_[1];						// 0x1A24
		int committing_accepted_[1];				// 0x1A28
		PartyJoinChallenge challenge_;				// 0x1A2C
	private:
		char pad_1A32[0x0006];						// 0x1A32
	public:
		PartyStateData party_state_data_;			// 0x1A38
		std::uint64_t lobby_id_;					// 0xB920
		std::uint32_t start_time_utc_;				// 0xB928
		int start_time_millis_;						// 0xB92C
		int timeout_time_;							// 0xB930
		int available_retry_count_;					// 0xB934
		PartyJoinResponse error_response_;			// 0xB938
		std::uint8_t error_reason_[1];				// 0xB93C
	private:
		char pad_B93D[0x0003];						// 0xB93D
	public:
		PartyJoinState error_state_;				// 0xB940
	private:
		char pad_B944[0x0004];						// 0xB944
	public:
		const char* error_message_;					// 0xB948
		bool should_go_to_mp_blade_;				// 0xB950
	private:
		char pad_B951[0x0007];						// 0xB951
	};
	ENGINE_ASSERT_SZ(PartyJoinInfo, 0xB958);
}
