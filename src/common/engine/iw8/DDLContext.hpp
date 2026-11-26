#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/DDLDef.hpp"
#include "engine/iw8/enums/DDLAccessOp.hpp"

namespace iw8 {
	class DDLContext {
	public:
		void* buff_;																				// 0x0000
		int len_;																					// 0x0008
	private:
		char pad_000C[0x0004];																		// 0x000C
	public:
		const DDLDef* def_;																			// 0x0010
		void(__fastcall* access_cb_)(const DDLContext*, std::uint32_t, std::uint32_t, DDLAccessOp);	// 0x0018
		void* user_data_;																			// 0x0020
		bool obfuscated_;																			// 0x0028
	private:
		char pad_0029[0x0003];																		// 0x0029
	public:
		std::uint32_t random_int_;																	// 0x002C
	};
	ENGINE_ASSERT_SZ(DDLContext, 0x0030);
}
