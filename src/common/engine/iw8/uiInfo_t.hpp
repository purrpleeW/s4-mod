#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/UiContext.hpp"
#include "engine/iw8/enums/uiMenuCommand_t.hpp"

namespace iw8 {
	class uiInfo_t {
	public:
		UiContext ui_dc_;					// 0x0000
		uiMenuCommand_t current_menu_type_;	// 0x0028
	};
	ENGINE_ASSERT_SZ(uiInfo_t, 0x002C);
}
