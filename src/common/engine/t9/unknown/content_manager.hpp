#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/unknown/content_pack.hpp"

namespace t9 {
	class content_manager {
	public:
		union {
			class {
			private:
				char pad_0000[0x0010];			// 0x0000
			public:
				int pack_count_;				// 0x0010
			private:
				char pad_0014[0x0004];			// 0x0014
			public:
				content_pack* content_packs_;	// 0x0018
			} v1_;

			class {
			private:
				char pad_0000[0x0008];			// 0x0000
			public:
				int pack_count_;				// 0x0008
			private:
				char pad_000C[0x0004];			// 0x000C
			public:
				content_pack* content_packs_;	// 0x0010
			} v2_;
		};

		int& get_pack_count();
		content_pack* get_content_pack(std::size_t index);
	};
	ENGINE_ASSERT_SZ(content_manager, 0x0020);
}