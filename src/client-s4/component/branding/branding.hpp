#pragma once
#include "engine/engine_common.hpp"

namespace branding {
	bool should_show_watermark();
	void update_watermark_setting(const std::string& new_watermark_setting);
}
