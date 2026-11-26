#pragma once
#include "engine/engine_common.hpp"

namespace iw9 {
	class LocalUserData {
	public:
		int sign_in_state_;					// 0x0000
		char gamertag_[36];					// 0x0004
		int gamertag_suffix_;				// 0x0028
		std::uint32_t hash_of_gamertag_;	// 0x002C
		char full_gamertag_[36];			// 0x0030
	private:
		char pad_0054[0x001C];				// 0x0054
	public:
		char platform_gamertag_[36];		// 0x0070
	private:
		char pad_0094[0x0004];				// 0x0094
	public:
		std::uint64_t xuid_;				// 0x0098
		char xuid_string_[21];				// 0x00A0
	private:
		char pad_00B5[0x0003];				// 0x00B5
	public:
		std::uint64_t platform_id_;			// 0x00B8
		char platform_id_string_[21];		// 0x00C0
	private:
		char pad_00D5[0x0043];				// 0x00D5
	public:
		void assign_username(const char* username, int suffix = 0);
		void assign_xuid(std::uint64_t xuid);
	};
}
