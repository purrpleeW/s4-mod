#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/DDLDef.hpp"
#include "engine/iw8/DDLMember.hpp"

namespace iw8 {
	class DDLState {
	public:
		bool is_valid_;			// 0x0000
	private:
		char pad_0001[0x0003];	// 0x0001
	public:
		int offset_;			// 0x0004
		int array_index_;		// 0x0008
	private:
		char pad_000C[0x0004];	// 0x000C
	public:
		DDLMember* member_;		// 0x0010
		const DDLDef* ddl_def_;	// 0x0018
	};
	ENGINE_ASSERT_SZ(DDLState, 0x0020);
}
