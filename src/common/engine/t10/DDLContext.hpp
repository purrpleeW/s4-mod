#pragma once

#include <cstddef>
#include <cstdint>

namespace t10 {
	class DDLDef;
	enum class DDLAccessOp : int {
		DDL_ACCESS_WRITE = 2
	};

	struct DDLContext {
		void* buff_;
		int len_;
		std::byte pad_000C[4];
		const DDLDef* def_;
		std::byte pad_0018[8];
		void(*access_cb_)(const DDLContext*, std::uint32_t, std::uint32_t, DDLAccessOp);
		void* user_data_;
		std::byte pad_0030[4];
		bool obfuscated_;
		std::byte pad_0035[3];
		std::uint32_t random_int_;
		std::byte pad_003C[4];
	};
	static_assert(sizeof(DDLContext) == 0x40);
	static_assert(offsetof(DDLContext, def_) == 0x10);
	static_assert(offsetof(DDLContext, access_cb_) == 0x20);
	static_assert(offsetof(DDLContext, user_data_) == 0x28);
	static_assert(offsetof(DDLContext, obfuscated_) == 0x34);
	static_assert(offsetof(DDLContext, random_int_) == 0x38);
}
