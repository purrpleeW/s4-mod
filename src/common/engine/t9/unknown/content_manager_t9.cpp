#if defined(_WIN64)
#include "engine/engine_common.hpp"
#include "engine/t9/unknown/content_manager.hpp"
#include "identification/game.hpp"

namespace t9 {
	bool cm_use_v1() {
		static bool use_v1 = identification::game::is_less_or_eq("0.01.2");
		return use_v1;
	}

	bool cm_use_v2() {
		static bool use_v2 = identification::game::is_less_or_eq("1.16.0");
		return use_v2;
	}

	bool cm_use_v3() {
		static bool use_v3 = identification::game::is_less_or_eq("1.23.3");
		return use_v3;
	}

	int& content_manager::get_pack_count() {
		if (cm_use_v1() || cm_use_v2()) {
			return this->v1_.pack_count_;
		}
		return this->v2_.pack_count_;
	}

	content_pack* content_manager::get_content_pack(std::size_t index) {
		if (cm_use_v1()) {
			return PTR_AS(content_pack*, PTR_AS(std::uintptr_t, this->v1_.content_packs_) + (index * 0xE8));
		}
		if (cm_use_v2()) {
			return PTR_AS(content_pack*, PTR_AS(std::uintptr_t, this->v1_.content_packs_) + (index * 0xD8));
		}
		if (cm_use_v3()) {
			return PTR_AS(content_pack*, PTR_AS(std::uintptr_t, this->v2_.content_packs_) + (index * 0xC8));
		}
		return PTR_AS(content_pack*, PTR_AS(std::uintptr_t, this->v2_.content_packs_) + (index * 0xD0));
	}
}
#endif
