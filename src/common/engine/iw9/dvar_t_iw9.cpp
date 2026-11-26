#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/iw9/dvar_t.hpp"
#include "identification/game.hpp"

namespace iw9 {
	bool dvar_use_v1() {
		static auto res = identification::game::is_less_or_eq("1.17.0");
		return res;
	}

	bool dvar_use_v2() {
		static auto res = identification::game::is_less_or_eq("1.18.1");
		return res;
	}

	DvarValue& dvar_t::get_current() {
		if (dvar_use_v1()) {
			return this->v1_.current_;
		}
		if (dvar_use_v2()) {
			return this->v2_.current_;
		}
		return this->v3_.current_;
	}

	DvarValue& dvar_t::get_reset() {
		if (dvar_use_v1()) {
			return this->v1_.reset_;
		}
		if (dvar_use_v2()) {
			return this->v2_.reset_;
		}
		return this->v3_.reset_;
	}

	DvarLimits& dvar_t::get_domain() {
		if (dvar_use_v1()) {
			return this->v1_.domain_;
		}
		if (dvar_use_v2()) {
			return this->v2_.domain_;
		}
		return this->v3_.domain_;
	}
}
#endif
