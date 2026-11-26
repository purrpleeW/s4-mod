#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdString.hpp"
#include "engine/iw8/enums/bdJSONType.hpp"

namespace iw8 {
	class bdJSONValue {
	public:
		bdJSONType type_;			// 0x0000
	private:
		char pad_0004[0x0004];		// 0x0004
	public:
		union {
			bdString* string_;		// 0x0008
			bdJSONObject* object_;	// 0x0008
			bdJSONArray* array_;	// 0x0008
			void* data_;			// 0x0008
		};
	};
	ENGINE_ASSERT_SZ(bdJSONValue, 0x0010);
}
