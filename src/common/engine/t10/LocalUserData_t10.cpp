#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/t10/LocalUserData.hpp"
#include "identification/game.hpp"
#include "utils/cryptography.hpp"
#include "utils/string.hpp"

namespace t10 {
	void LocalUserData::assign_username(const char* username, int suffix) {
		char username_truncated[0x24 + 1]{};
		auto username_len = std::min(strlen(username), sizeof(username_truncated) - 1);

		this->gamertag_suffix_ = suffix;
		this->hash_of_gamertag_ = utils::cryptography::platform_usernames::compute(username, username_len);

		utils::string::copy(username_truncated, sizeof(username_truncated) - 1, username, true);

		utils::string::copy(this->gamertag_, username_truncated, true);
		utils::string::copy(this->full_gamertag_, username_truncated, true);
		utils::string::copy(this->platform_gamertag_, username_truncated, true);
	}

	void LocalUserData::assign_xuid(std::uint64_t xuid) {
		const char* xuid_string = utils::string::va("%Iu", xuid);

		this->xuid_ = xuid;
		utils::string::copy(this->xuid_string_, xuid_string, true);

		this->platform_id_ = xuid;
		utils::string::copy(this->platform_id_string_, xuid_string, true);
	}
}
#endif
