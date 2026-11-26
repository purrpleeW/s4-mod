#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class scrContext_t {
	private:
		char pad_00000000[0x00000C74];	// 0x00000000
	public:
		std::uint32_t func_begin_;		// 0x00000C74
		std::uint32_t func_end_;		// 0x00000C78
		std::uint32_t func_count_;		// 0x00000C7C
		std::uint32_t meth_begin_;		// 0x00000C80
		std::uint32_t meth_end_;		// 0x00000C84
		std::uint32_t meth_count_;		// 0x00000C88
	private:
		char pad_00000C8C[0x00000004];	// 0x00000000
	public:
		void** func_table_;				// 0x00000C90
		void** meth_table_;				// 0x00000C98
	};
}
