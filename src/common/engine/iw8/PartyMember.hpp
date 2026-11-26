#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/ClientAuthoritativeMemberInfo.hpp"
#include "engine/iw8/NetConnection.hpp"
#include "engine/iw8/XUID.hpp"

namespace iw8 {
	class PartyMember {
	public:
		ClientAuthoritativeMemberInfo info;		// 0x0000
		// 8 bytes were removed in the following
		/*
		NomineeInfo migrate_nominee_info_;		//
		char challenge_[6];						//
		bool voice_registered_;					//
		bool muted_;							//
		int last_voice_packet_received_time_;	//
		XUID challenge_request_xuid_;			//
		*/
	private:
		char pad_00B8[0x0030];					// 0x00B8
	public:
		NetConnection connection_;				// 0x00E8
		int last_packet_time_;					// 0x0108
		int last_heartbeat_time_;				// 0x010C
		int last_party_state_time_;				// 0x0110
		int last_party_state_ack_;				// 0x0114
		int last_match_rules_ack_;				// 0x0118
		int last_party_go_time_;				// 0x011C
		int nat_type_;							// 0x0120
	private:
		// bitarray<256> ackedMembers;
		char pad_0124[0x0020];					// 0x0124
		// bitarray<256> connectivityAckedMembers;
		char pad_0144[0x0020];					// 0x0144
		// bitarray<256> otherDataAckedMembers;
		char pad_0164[0x0020];					// 0x0164
	public:
		int subparty_index_;					// 0x0184
		int team_;								// 0x0188
		int squad_;								// 0x018C
		std::uint16_t score_;					// 0x0190
		std::uint8_t status_;					// 0x0192
		std::uint8_t join_type_;				// 0x0193
		int deaths_;							// 0x0194
		XUID player_uid_;						// 0x0198
		// 8 bytes were removed in the following
		/*
		XUID player_updated_uid_;				// 0x01A0
		int is_guest_account_;					// 0x01A8
		int migrate_ping_time_;					// 0x01AC
		int migrate_ping_;						// 0x01B0
		int migrate_ping_count_;				// 0x01B4
		int connectivity_bits_;					// 0x01B8
		int next_connectivity_test_time_[2];	// 0x01BC
		int hosting_attempt_time_[7];			// 0x01C4
		bool hosting_attempted_[7];				// 0x01E0
		bool migrate_nominated_;				// 0x01E7
		bool migrate_heard_from_;				// 0x01E8
		bool is_creating_party_chat_;			// 0x01E9
		bool is_user_invite_joining_;			// 0x01EA
	private:
		char pad_01EB[0x0005];					// 0x01EB
		*/
	private:
		char pad_01A0[0x0048];					// 0x01A0
	};
	ENGINE_ASSERT_SZ(PartyMember, 0x01E8);
}
