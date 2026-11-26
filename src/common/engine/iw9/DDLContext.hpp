#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/DDLDef.hpp"
#include "engine/iw9/enums/DDLAccessOp.hpp"

namespace iw9 {
	class DDLContext {
	public:
		void* buff_;																					// 0x0000
		int len_;																						// 0x0008
	private:
		char pad_000C[0x0004];																			// 0x000C
	public:
		const DDLDef* def_;																				// 0x0010
	private:
		char pad_0018[0x0008];																			// 0x0018
	public:
		void (__fastcall* access_cb_)(const DDLContext*, std::uint32_t, std::uint32_t, DDLAccessOp);	// 0x0020
		void* user_data_;																				// 0x0028
	private:
		char pad_0030[0x0004];																			// 0x0030
	public:
		bool obfuscated_;																				// 0x0034
	private:
		char pad_0035[0x0003];																			// 0x0035
	public:
		std::uint32_t random_int_;																		// 0x0038
	private:
		char pad_003C[0x0004];																			// 0x003C
	};
	ENGINE_ASSERT_SZ(DDLContext, 0x0040);
}
