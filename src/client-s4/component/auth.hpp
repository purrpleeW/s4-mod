#pragma once
#include "common.hpp"
#include <engine/s4/LocalUserData.hpp>
#include <engine/s4/XUID.hpp>

namespace auth {
	inline s4::XUID user_xuid{};

	s4::LocalUserData* get_local_user_data();
	void on_stats_init(std::function<void()> callback);
}
