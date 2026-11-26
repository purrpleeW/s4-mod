#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class HksCompilerSettings {
	public:
		enum class BytecodeSharingFormat {
			BYTECODE_DEFAULT = 0,
			BYTECODE_INPLACE = 1,
			BYTECODE_REFERENCED = 2
		};

		enum class IntLiteralOptions {
			INT_LITERALS_NONE = 0,
			INT_LITERALS_LUD = 1,
			INT_LITERALS_32BIT = 1,
			INT_LITERALS_UI64 = 2,
			INT_LITERALS_64BIT = 2,
			INT_LITERALS_ALL = 3
		};

		hksBool emit_struct_code_;						// 0x0000
	private:
		char pad_0004[0x0004];							// 0x0004
	public:
		const hksChar** strip_names_;					// 0x0008
		BytecodeSharingFormat bytecode_sharing_format_;	// 0x0010
		IntLiteralOptions enable_int_literals_;			// 0x0014
		hks_debug_map debug_map_;						// 0x0018
	};
	ENGINE_ASSERT_SZ(HksCompilerSettings, 0x0020);
}
