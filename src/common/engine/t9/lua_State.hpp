#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/enums/HksError.hpp"
#include "engine/t9/hks/ApiStack.hpp"
#include "engine/t9/hks/CallStack.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"
#include "engine/t9/hks/DebugHook.hpp"
#include "engine/t9/hks/HksGlobal.hpp"
#include "engine/t9/hks/UpValue.hpp"

namespace t9 {
	class lua_State : public hks::ChunkHeader {
	public:
		enum class Status {
			NEW = 1,
			RUNNING = 2,
			YIELDED = 3,
			DEAD_ERROR = 4
		};

		hks::HksGlobal* global_;		// 0x0010
		hks::CallStack call_stack_;		// 0x0018
		hks::ApiStack api_stack_;		// 0x0048
		hks::UpValue* pending_;			// 0x0068
		HksObject globals_;				// 0x0070
		HksObject c_env_;				// 0x0080
		hks::ErrorHandler call_sites_;	// 0x0090
		hksInt32 number_of_c_calls_;	// 0x0098
	private:
		char pad_009C[0x0004];			// 0x009C
	public:
		void* context_;					// 0x00A0
		hks::InternString* name_;		// 0x00A8
		lua_State* next_;				// 0x00B0
		lua_State* next_state_stack_;	// 0x00B8
		Status status_;					// 0x00C0
		HksError error_;				// 0x00C4
		hks::DebugHook debug_hook_;		// 0x00C8
	};
	ENGINE_ASSERT_SZ(lua_State, 0x00E8);
}
