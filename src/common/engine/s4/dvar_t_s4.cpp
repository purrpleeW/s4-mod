#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/s4/dvar_t.hpp"
#include "identification/game.hpp"

namespace s4 {
	bool dv_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("1.16.0");
		return use_v1;
	}

	DvarValue& dvar_t::get_current() {
		if (dv_use_v1()) {
			return this->v1_.current_;
		}
		return this->v2_.current_;
	}

	DvarLimits& dvar_t::get_domain() {
		if (dv_use_v1()) {
			return this->v1_.domain_;
		}
		return this->v2_.domain_;
	}

	DvarValue& dvar_t::get_reset() {
		if (dv_use_v1()) {
			return this->v1_.reset_;
		}
		return this->v2_.reset_;
	}

	DvarType& dvar_t::get_type() {
		if (dv_use_v1()) {
			return this->v1_.type_;
		}
		return this->v2_.type_;
	}
}
#endif
