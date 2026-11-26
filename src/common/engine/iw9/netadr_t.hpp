#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/enums/netadrtype_t.hpp"
#include "engine/iw9/enums/netsrc_t.hpp"

namespace iw9 {
	class netadr_t {
	public:
		netadrtype_t type_;		// 0x0000
		std::uint8_t ip_[4];	// 0x0004
		std::uint16_t port_;	// 0x0008
		std::uint16_t flags_;	// 0x000A
		netsrc_t local_net_id_;	// 0x000C
		int addr_handle_index_;	// 0x0010

		bool matches_type_ignorant(const netadr_t& other) const {
			return DEREF_PTR_AS(const std::uint32_t, this->ip_) == DEREF_PTR_AS(const std::uint32_t, other.ip_) && this->port_ == other.port_;
		}

		std::string to_string_type_ignorant() const {
			return std::format("{:d}.{:d}.{:d}.{:d}:{:d}", this->ip_[0], this->ip_[1], this->ip_[2], this->ip_[3], this->port_);
		}
	};
	ENGINE_ASSERT_SZ(netadr_t, 0x0014);
}
