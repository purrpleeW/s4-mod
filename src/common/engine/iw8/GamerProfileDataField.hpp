#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/GamerProfileDataType.hpp"

namespace iw8 {
	class GamerProfileDataField {
	public:
		const char* setting_name_;		// 0x0000
		std::uintptr_t offset_;			// 0x0008
		GamerProfileDataType type_;		// 0x0010
		int size_;						// 0x0014
		float default_val_;				// 0x0018
		float min_val_;					// 0x001C
		float max_val_;					// 0x0020
		bool is_game_option_;			// 0x0024
	private:
		char pad_0025[0x0003];			// 0x0025
	public:
		int bit_shift_;					// 0x0028
	private:
		char pad_002C[0x0004];			// 0x002C
	};
	ENGINE_ASSERT_SZ(GamerProfileDataField, 0x0030);
}
