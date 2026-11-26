#pragma once
#include "common.hpp" 
#include "game/game.hpp"

namespace network {
	using callback = std::function<void(const s4::netadr_t&, const std::string_view&, const int)>;

	inline constexpr int query_socket_port_min = 26016;
	inline constexpr int query_socket_port_max = 26025;
	inline SOCKET query_socket = INVALID_SOCKET;
	inline std::uint16_t query_socket_port = 0;
	inline std::string last_error_code = "";
	inline std::string last_error_message = "";
	inline bool should_report_error = false;

	void on(const std::string& command, const callback& callback);
	void send(const s4::netadr_t& address, const std::string& command, const std::string& data = {}, char separator = ' ', std::optional<s4::netadrtype_t> adr_type_override = std::nullopt);
	void send_data(const s4::netadr_t& address, const std::string& data, std::optional<s4::netadrtype_t> adr_type_override = std::nullopt);

	bool are_addresses_equal(const s4::netadr_t& a, const s4::netadr_t& b);

	const char* net_adr_to_string(const s4::netadr_t& a);
}

inline bool operator==(const s4::netadr_t& a, const s4::netadr_t& b) {
	return network::are_addresses_equal(a, b);
}

inline bool operator!=(const s4::netadr_t& a, const s4::netadr_t& b) {
	return !(a == b);
}

namespace std {
	template <>
	struct equal_to<s4::netadr_t> {
		using result_type = bool;

		bool operator()(const s4::netadr_t& lhs, const s4::netadr_t& rhs) const {
			return network::are_addresses_equal(lhs, rhs);
		}
	};

	template <>
	struct hash<s4::netadr_t> {
		std::size_t operator()(const s4::netadr_t& x) const noexcept {
			return hash<std::uint32_t>()(DEREF_PTR_AS(const std::uint32_t, &x.ip_[0])) ^ hash<std::uint16_t>()(x.port_);
		}
	};
}
