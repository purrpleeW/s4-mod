#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw8/LocalUserData.hpp"
#include "identification/game.hpp"
#include "utils/cryptography.hpp"
#include "utils/string.hpp"

namespace iw8 {
	bool ud_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("1.23.2") || identification::game::is("1.20.4-replay");
		return use_v1;
	}

	bool ud_use_v2() {
		static bool use_v2 = identification::game::is_less("1.46.0");
		return use_v2;
	}

	std::uint32_t& LocalUserData::get_hash_of_gamertag() {
		if (ud_use_v1()) {
			return this->v1_.hash_of_gamertag_;
		}
		if (ud_use_v2()) {
			return this->v2_.hash_of_gamertag_;
		}
		return this->v3_.hash_of_gamertag_;
	}

	char (*LocalUserData::get_full_gamertag_ptr())[64] {
		if (ud_use_v1()) {
			return &this->v1_.full_gamertag_;
		}
		if (ud_use_v2()) {
			return &this->v2_.full_gamertag_;
		}
		return &this->v3_.full_gamertag_;
	}

	char (*LocalUserData::get_platform_gamertag_ptr())[36] {
		if (ud_use_v1()) {
			return &this->v1_.platform_gamertag_;
		}
		if (ud_use_v2()) {
			return &this->v2_.platform_gamertag_;
		}
		return &this->v3_.platform_gamertag_;
	}

	char (*LocalUserData::get_xuid_string_ptr())[21] {
		if (ud_use_v1()) {
			return &this->v1_.xuid_string_;
		}
		if (ud_use_v2()) {
			return &this->v2_.xuid_string_;
		}
		return &this->v3_.xuid_string_;
	}

	char (*LocalUserData::get_platform_id_string_ptr())[21] {
		if (ud_use_v1()) {
			return &this->v1_.platform_id_string_;
		}
		if (ud_use_v2()) {
			return &this->v2_.platform_id_string_;
		}
		return &this->v3_.platform_id_string_;
	}

	std::uint64_t& LocalUserData::get_xuid() {
		if (ud_use_v1()) {
			return this->v1_.xuid_;
		}
		if (ud_use_v2()) {
			return this->v2_.xuid_;
		}
		return this->v3_.xuid_;
	}

	std::uint64_t& LocalUserData::get_platform_id() {
		if (ud_use_v1()) {
			return this->v1_.platform_id_;
		}
		if (ud_use_v2()) {
			return this->v2_.platform_id_;
		}
		return this->v3_.platform_id_;
	}

	void LocalUserData::set_xuid(std::uint64_t xuid) {
		auto xuid_string = std::format("{}", xuid);

		this->get_xuid() = xuid;
		utils::string::copy(*this->get_xuid_string_ptr(), xuid_string.c_str(), true);
	}

	void LocalUserData::set_platform_id(std::uint64_t platform_id) {
		auto platform_id_string = std::format("{}", platform_id);

		this->get_platform_id() = platform_id;
		utils::string::copy(*this->get_platform_id_string_ptr(), platform_id_string.c_str(), true);
	}

	void LocalUserData::assign_username(const char* username, int suffix) {
		char username_truncated[0x24 + 1]{};
		auto username_len = std::min(strlen(username), sizeof(username_truncated) - 1);

		if (!ud_use_v1()) {
			if (ud_use_v2()) {
				this->v2_.gamertag_suffix_ = suffix;
			}
			else {
				this->v3_.gamertag_suffix_ = suffix;
			}
		}
		this->get_hash_of_gamertag() = utils::cryptography::platform_usernames::compute(username, username_len);

		utils::string::copy(username_truncated, sizeof(username_truncated) - 1, username, true);

		utils::string::copy(this->gamertag_, username_truncated, true);
		utils::string::copy(*this->get_full_gamertag_ptr(), username_truncated, true);
		utils::string::copy(*this->get_platform_gamertag_ptr(), utils::string::va("%s#%d", username_truncated, suffix), true);
	}

	void LocalUserData::assign_xuid(std::uint64_t xuid) {
		this->set_xuid(xuid);
		this->set_platform_id(xuid);
	}
}
#endif
