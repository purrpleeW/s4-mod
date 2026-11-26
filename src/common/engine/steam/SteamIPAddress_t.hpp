#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/enums/ESteamIPType.hpp"

namespace steam {
	class SteamIPAddress_t {
	public:
		union {
			std::uint32_t ipv4_;			// 0x0000
			std::uint8_t ipv6_[16];			// 0x0000
			std::uint64_t ipv6_qword_[2];	// 0x0000
		};
		ESteamIPType type_;					// 0x0010

		bool is_set() const {
			if (this->type_ == ESteamIPType::k_ESteamIPTypeIPv4) {
				return this->ipv4_ != 0;
			}
			else {
				return this->ipv6_qword_[0] !=0 || this->ipv6_qword_[1] != 0;
			}
		}

		static SteamIPAddress_t ipv4_any() {
			SteamIPAddress_t ip_out;
			ip_out.type_ = ESteamIPType::k_ESteamIPTypeIPv4;
			ip_out.ipv4_ = 0;
			return ip_out;
		}

		static SteamIPAddress_t ipv6_any() {
			SteamIPAddress_t ip_out;
			ip_out.type_ = ESteamIPType::k_ESteamIPTypeIPv6;
			ip_out.ipv6_qword_[0] = 0;
			ip_out.ipv6_qword_[1] = 0;
			return ip_out;
		}

		static SteamIPAddress_t ipv4_loopback() {
			SteamIPAddress_t ip_out;
			ip_out.type_ = ESteamIPType::k_ESteamIPTypeIPv4;
			ip_out.ipv4_ = 0x7F000001;
			return ip_out;
		}

		static SteamIPAddress_t ipv6_loopback() {
			SteamIPAddress_t ip_out;
			ip_out.type_ = ESteamIPType::k_ESteamIPTypeIPv6;
			ip_out.ipv6_qword_[0] = 0;
			ip_out.ipv6_qword_[1] = 0;
			ip_out.ipv6_[15] = 1;
			return ip_out;
		}
	};
}
