#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw8/dvar_t.hpp"
#include "identification/game.hpp"

namespace iw8 {
	std::uint32_t& dvar_t::get_checksum() {
		if (identification::game::is("1.20.4-replay")) {
			return this->replay_.checksum_;
		}
		return this->ship_.checksum_;
	}

	std::uint8_t& dvar_t::get_type() {
		if (identification::game::is("1.20.4-replay")) {
			return this->replay_.type_;
		}
		return this->ship_.type_;
	}

	DvarValue& dvar_t::get_current() {
		if (identification::game::is("1.20.4-replay")) {
			return this->replay_.current_;
		}
		return this->ship_.current_;
	}

	DvarValue& dvar_t::get_reset() {
		if (identification::game::is("1.20.4-replay")) {
			return this->replay_.reset_;
		}
		return this->ship_.reset_;
	}

	DvarLimits& dvar_t::get_domain() {
		if (identification::game::is("1.20.4-replay")) {
			return this->replay_.domain_;
		}
		return this->ship_.domain_;
	}
}
#endif
