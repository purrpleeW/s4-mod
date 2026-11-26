#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdSockAddr {
	public:
		bdSockAddr(std::uint32_t addr = 0) {
			this->in_un_.sockaddr_storage_.ss_family = 2;
			this->in_un_.ipv4_sockaddr_.sin_addr.S_un.S_addr = addr;
			this->in_un_.ipv4_sockaddr_.sin_port = htons(0);
		}

		std::uint16_t get_port() {
			if (this->in_un_.sockaddr_storage_.ss_family == 23 || this->in_un_.sockaddr_storage_.ss_family == 2) {
				return ntohs(this->in_un_.ipv4_sockaddr_.sin_port);
			}
			return 0;
		}

		bool compare(bdSockAddr other, bool compare_ports) {
			bool res = false;
			if (this->in_un_.sockaddr_storage_.ss_family == other.in_un_.sockaddr_storage_.ss_family) {
				res = this->in_un_.ipv4_sockaddr_.sin_addr.S_un.S_addr == other.in_un_.ipv4_sockaddr_.sin_addr.S_un.S_addr;
			}

			if (!compare_ports) {
				return res;
			}

			return res && this->get_port() == other.get_port();
		}

		static inline bdSockAddr loopback(std::uint16_t family, std::uint16_t port) {
			bdSockAddr result;

			result.in_un_.sockaddr_storage_.ss_family = 2;
			result.in_un_.ipv4_sockaddr_.sin_addr.S_un.S_addr = 0xFF00FF00;
			htons(0);
			if (family == 23) {
				result.in_un_.sockaddr_storage_.ss_family = 23;
				result.in_un_.ipv4_sockaddr_.sin_port = htons(port);
				inet_pton(2, "::1", &result.in_un_.ipv6_sockaddr_.sin6_flowinfo);
			}
			else {
				result.in_un_.sockaddr_storage_.ss_family = 2;
				result.in_un_.ipv4_sockaddr_.sin_addr.S_un.S_addr = 0x0100007F;	// 127.0.0.1
				result.in_un_.ipv4_sockaddr_.sin_port = htons(port);
			}

			return result;
		}

		union {
			sockaddr_storage sockaddr_storage_;	// 0x0000
			sockaddr_in ipv4_sockaddr_;			// 0x0000
			sockaddr_in6 ipv6_sockaddr_;		// 0x0000
		} in_un_;								// 0x0000
	};
	ENGINE_ASSERT_SZ(bdSockAddr, 0x0080);
}
