#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class clientUIActive_t {
	public:
		bool active_;
		bool is_running_;
	private:
		char pad_0002[0x0002];
	public:
		int key_catchers_;
	};
}
