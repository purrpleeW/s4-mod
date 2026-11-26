#include "engine/engine_common.hpp"

namespace iw8 {
	enum class DvarSetSource {
		DVAR_SOURCE_INTERNAL = 0,
		DVAR_SOURCE_EXTERNAL = 1,
		DVAR_SOURCE_SCRIPT = 2,
		DVAR_SOURCE_UISCRIPT = 3,
		DVAR_SOURCE_SERVERCMD = 4,
		DVAR_SOURCE_DEVGUI = 5,
		DVAR_SOURCE_NUM = 6
	};
}
