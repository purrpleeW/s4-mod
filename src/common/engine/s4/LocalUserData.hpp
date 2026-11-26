#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/enums/CoDAuthState.hpp"
#include "utils/cryptography.hpp"
#include "utils/string.hpp"

namespace s4 {
	class LocalUserData {
	public:
		CoDAuthState sign_in_state_;			// 0x0000
		char gamertag_[36];						// 0x0004
		int gamertag_suffix_;					// 0x0028
		int hash_of_gamertag_;					// 0x002C
		char platform_gamertag_[64];			// 0x0030
		char full_gamertag_[36];				// 0x0070
	private:
		char pad_0094[0x0004];					// 0x0094
	public:
		std::uint64_t xuid_;					// 0x0098
		char xuid_string_[21];					// 0x00A0
	private:
		char pad_00B5[0x0003];					// 0x00B5
	public:
		std::uint64_t platform_id_;				// 0x00B8
		char platform_id_string_[21];			// 0x00C0
	private:
		char pad_00D5[0x0003];					// 0x00D5
	public:
		void assign_username(const char* username, int suffix = 0) {
			char username_truncated[0x24 + 1]{};
			auto username_len = std::min(strlen(username), sizeof(username_truncated) - 1);

			this->gamertag_suffix_ = suffix;
			this->hash_of_gamertag_ = utils::cryptography::platform_usernames::compute(username, username_len);

			utils::string::copy(username_truncated, sizeof(username_truncated) - 1, username, true);

			utils::string::copy(this->gamertag_, username_truncated, true);
			utils::string::copy(this->full_gamertag_, username_truncated, true);
			utils::string::copy(this->platform_gamertag_, utils::string::va("%s#%d", username_truncated, suffix), true);
		}

		void assign_xuid(std::uint64_t xuid) {
			const char* xuid_string = utils::string::va("%Iu", xuid);

			this->xuid_ = xuid;
			utils::string::copy(this->xuid_string_, xuid_string, true);

			this->platform_id_ = xuid;
			utils::string::copy(this->platform_id_string_, xuid_string, true);
		}
	};
	ENGINE_ASSERT_SZ(LocalUserData, 0x00D8);
}
