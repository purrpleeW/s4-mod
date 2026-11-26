#pragma once
#include "game/function_types.hpp"
#include <memory/signature_store.hpp>

namespace patches::beta_auth {
	void post_load();
	void find_signatures(memory::signature_store& batch, functions::PlatformAuth_GetUsernameT*& get_username, functions::PlatformAuth_GetXUIDT*& get_xuid);
	void find_signatures_bnet(memory::signature_store& batch);
	bool should_activate();
	bool should_activate_bnet();
	void post_unpack();
	void finish_auth(s4::BattleNetAuth* auth);
	void sign_in_profile();
	void frame(void (*initialize_stats)());
}
