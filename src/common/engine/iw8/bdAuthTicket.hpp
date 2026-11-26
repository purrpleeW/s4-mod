#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdAuthTicket {
	public:
		enum class bdTicketTypes {
			BD_AUTH_USER_TO_SERVICE_TICKET = 0,
			BD_AUTH_HOST_TO_SERVICE_TICKET = 1,
			BD_AUTH_USER_TO_HOST_TICKET = 2
		};

		std::uint32_t magic_number_;				// 0x0000
		std::uint8_t type_;							// 0x0004
	private:
		char pad_0005[0x0003];						// 0x0005
	public:
		std::uint32_t title_id_;					// 0x0008
		std::uint32_t time_issued_;					// 0x000C
		std::uint32_t time_expires_;				// 0x0010
	private:
		char pad_0014[0x0004];						// 0x0014
	public:
		std::uint64_t license_id_;					// 0x0018
		std::uint64_t user_id_;						// 0x0020
		char username_[64];							// 0x0028
		std::uint8_t session_key_[24];				// 0x0068
		std::uint8_t using_hash_magic_number_[3];	// 0x0080
		std::uint8_t hash_[4];						// 0x0083
	private:
		char pad_0087[0x0001];						// 0x0087
	};
}
