#include "ClCommandInfo.hpp"
#include <identification/game.hpp>

namespace iw8 {
	ClCommandInfo* ClCommandInfo::get_next() {
		static bool is_replay = identification::game::is_config(identification::game::Replay);
		return CLASS_VALUE_AT_PTR(this, (is_replay ? 0x0038 : 0x0028), ClCommandInfo);
	}
}