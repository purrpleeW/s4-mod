#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/t9/unknown/content_pack.hpp"
#include "identification/game.hpp"

namespace t9 {
	bool cp_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("0.01.2");
		return use_v1;
	}

	bool cp_use_v2() {
		static bool use_v2 = identification::game::is_less_or_eq("1.16.0");
		return use_v2;
	}

	bool cp_use_v3() {
		static bool use_v3 = identification::game::is_less_or_eq("1.23.3");
		return use_v3;
	}

	void content_pack::unlock() {
		if (cp_use_v1()) {
			this->v1_.var1_ = 0;
			this->v1_.var2_ = 0;
			this->v1_.var3_ = 2;
			this->v1_.var4_ = 3;
		}
		else if (cp_use_v2()) {
			this->v2_.var1_ = 0;
			this->v2_.var2_ = 0;
			this->v2_.var3_ = 2;
			this->v2_.var4_ = 3;
		}
		else if (cp_use_v3()) {
			this->v3_.var1_ = 0;
			this->v3_.var2_ = 2;
			this->v3_.var3_ = 3;
		}
		else {
			this->v4_.var1_ = 0;
			this->v4_.var2_ = 2;
			this->v4_.var3_ = 3;
		}
	}
}
#endif
