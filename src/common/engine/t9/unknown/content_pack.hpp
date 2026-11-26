#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class content_pack {
	public:
		union {
			class {
			private:
				char pad_0000[0x0024];	// 0x0000
			public:
				int var1_;				// 0x0024
				int var2_;				// 0x0028
			private:
				char pad_002C[0x0034];	// 0x002C
			public:
				int var3_;				// 0x0060
				int var4_;				// 0x0064
			private:
				char pad_0058[0x0080];	// 0x0068
			} v1_;

			class {
			private:
				char pad_0000[0x0024];	// 0x0000
			public:
				int var1_;				// 0x0024
				int var2_;				// 0x0028
			private:
				char pad_002C[0x0024];	// 0x002C
			public:
				int var3_;				// 0x0050
				int var4_;				// 0x0054
			private:
				char pad_0058[0x0080];	// 0x0058
			} v2_;

			class {
			private:
				char pad_0000[0x0014];	// 0x0000
			public:
				int var1_;				// 0x0014
			private:
				char pad_0018[0x0028];	// 0x0018
			public:
				int var2_;				// 0x0040
				int var3_;				// 0x0044
			private:
				char pad_0048[0x0080];	// 0x0048
			} v3_;

			class {
			private:
				char pad_0000[0x0014];	// 0x0000
			public:
				int var1_;				// 0x0014
			private:
				char pad_0018[0x0030];	// 0x0018
			public:
				int var2_;				// 0x0048
				int var3_;				// 0x004C
			private:
				char pad_0050[0x0080];	// 0x0050
			} v4_;
		};

		void unlock();
	};
	ENGINE_ASSERT_SZ(content_pack, 0x00E8);
}