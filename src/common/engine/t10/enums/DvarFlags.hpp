#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	enum class DvarFlags : std::uint32_t {
		DVAR_NOFLAG					= 0,
		DVAR_ARCHIVE				= 1 << 0,
		DVAR_LATCH					= 1 << 1,
		DVAR_CHEAT					= 1 << 2,
		DVAR_CODINFO				= 1 << 3,
		DVAR_SCRIPTINFO				= 1 << 4,
		DVAR_TEMP					= 1 << 5,
		DVAR_SAVED					= 1 << 6,
		DVAR_INTERNAL				= 1 << 7,
		DVAR_EXTERNAL				= 1 << 8,
		DVAR_USERINFO				= 1 << 9,
		DVAR_SERVERINFO				= 1 << 10,
		DVAR_ROM					= 1 << 11,
		DVAR_SYSTEMINFO				= 1 << 12,
		DVAR_INIT					= 1 << 13,
		DVAR_FLAG_CHANGEABLE_RESET	= 1 << 14,
		DVAR_FLAG_AUTOEXEC			= 1 << 15,
		DVAR_UNADDABLE_FLAGS		= DVAR_LATCH | DVAR_CHEAT | DVAR_EXTERNAL | DVAR_ROM | DVAR_INIT
	};
	DECLARE_BITWISE_OPS(DvarFlags);
}
