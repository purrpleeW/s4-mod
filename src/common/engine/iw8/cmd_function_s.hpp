#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class cmd_function_s {
	public:
		cmd_function_s* next_;							// 0x0000
		const char* name_;								// 0x0008

		union {
			struct {
				const char** auto_complete_list_;		// 0x0010
				unsigned int auto_complete_list_count_;	// 0x0018
			private:
				char pad_001C[0x0004];					// 0x001C
			public:
				void(__fastcall* function_)();			// 0x0020
			} replay_;

			struct {
				void(__fastcall* function_)();			// 0x0010
			} ship_;
		};
	};
	ENGINE_ASSERT_SZ(cmd_function_s, 0x0028);
}
