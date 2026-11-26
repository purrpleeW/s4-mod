#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksCompilerSettings.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/enums/HksBytecodeSharingMode.hpp"
#include "engine/t9/enums/HksBytecodeEndianness.hpp"
#include "engine/t9/hks/ChunkList.hpp"
#include "engine/t9/hks/DebugInstance.hpp"
#include "engine/t9/hks/GarbageCollector.hpp"
#include "engine/t9/hks/MemoryManager.hpp"
#include "engine/t9/hks/RuntimeProfileData.hpp"
#include "engine/t9/hks/StaticStringCache.hpp"
#include "engine/t9/hks/StringTable.hpp"
#include "engine/t9/lua_plus/LuaObject.hpp"

namespace t9::hks {
	class HksGlobal {
	public:
		MemoryManager memory_;								// 0x0000
		GarbageCollector collector_;						// 0x0058
		StringTable string_table_;							// 0x01C0
		HksBytecodeSharingMode bytecode_sharing_mode_;		// 0x01D8
	private:
		char pad_01DC[0x0004];								// 0x01DC
	public:
		HksObject registry_;								// 0x01E0
		ChunkList user_data_list_;							// 0x01F0
		lua_State* root_;									// 0x0200
		StaticStringCache static_string_cache_;				// 0x0208
		DebugInstance* debugger_;							// 0x04A8
		void* profiler_;									// 0x04B0
		RuntimeProfileData run_profiler_data_;				// 0x04B8
		HksCompilerSettings compiler_settings_;				// 0x0568
		lua_CFunction panic_function_;						// 0x0588
		LuaPlus::LuaObject* luaplus_object_list_;			// 0x0590
		int heap_assertion_frequency_;						// 0x0598
		int heap_assertion_count_;							// 0x059C
		HksLogFunc log_function_;							// 0x05A0
		HksEmergencyGCFailFunc emergency_gc_fail_function_;	// 0x05A8
		HksBytecodeEndianness bytecode_dump_endianness_;	// 0x05B0
	private:
		char pad_05B4[0x0004];								// 0x05B4
	};
	ENGINE_ASSERT_SZ(HksGlobal, 0x05B8);
}
