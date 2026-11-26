#pragma once
#include "engine/engine_common.hpp"

namespace s2 {
	enum class HksObjectType {
		TANY = -2,
		TNONE = -1,
		TNIL = 0,
		TBOOLEAN = 1,
		TLIGHTUSERDATA = 2,
		TNUMBER = 3,
		TSTRING = 4,
		TTABLE = 5,
		TFUNCTION = 6,
		TUSERDATA = 7,
		TTHREAD = 8,
		TIFUNCTION = 9,
		TCFUNCTION = 10,
		TUI64 = 11,
		TSTRUCT = 12,
		NUM_TYPE_OBJECTS = 14
	};
}
