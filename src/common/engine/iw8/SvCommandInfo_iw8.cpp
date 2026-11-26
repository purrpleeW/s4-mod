#include "SvCommandInfo.hpp"
#include <identification/game.hpp>

namespace iw8 {
	SvCommandInfo* SvCommandInfo::get_next() {
		static bool is_replay = identification::game::is_config(identification::game::Replay);
		return CLASS_VALUE_AT_PTR(this, (is_replay ? 0x0060 : 0x0040), SvCommandInfo);
	}
}