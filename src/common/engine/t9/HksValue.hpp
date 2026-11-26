#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	union HksValue {
		hks::cclosure* c_closure_;		// 0x0000
		hks::HksClosure* closure_;		// 0x0000
		hks::UserData* user_data_;		// 0x0000
		hks::HashTable* table_;			// 0x0000
		hks::StructInst* t_struct_;		// 0x0000
		hks::InternString* str_;		// 0x0000
		lua_State* thread_;				// 0x0000
		void* ptr_;						// 0x0000
		HksNumber number_;				// 0x0000
		HksNativeValueAsInt native_;	// 0x0000
		hksInt32 boolean_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(HksValue, 0x0008);
}
