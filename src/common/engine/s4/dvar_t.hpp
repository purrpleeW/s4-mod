#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/enums/DvarType.hpp"
#include "engine/s4/DvarLimits.hpp"
#include "engine/s4/DvarValue.hpp"

namespace s4 {
	class dvar_t {
	public:
		union {
			class {
			public:
				std::uint64_t name_hash_;	// 0x0000
				dvar_t* next_;				// 0x0008
				std::uint32_t flags_;		// 0x0010
				std::uint8_t level_[1];		// 0x0014
				DvarType type_;				// 0x0015
				bool modified_;				// 0x0016
			private:
				char pad_0017[0x0001];		// 0x0017
			public:
				DvarValue current_;			// 0x0018
			private:
				char pad_0028[0x0030];		// 0x0028
			public:
				DvarValue reset_;			// 0x0058
			private:
				char pad_0068[0x0010];		// 0x0068
			public:
				DvarLimits domain_;			// 0x0078
			} v1_;

			class {
			public:
				DvarValue current_;			// 0x0000
			private:
				char pad_0010[0x0030];		// 0x0010
			public:
				DvarValue reset_;			// 0x0040
			private:
				char pad_0050[0x0010];		// 0x0050
			public:
				std::uint64_t name_hash_;	// 0x0060
				dvar_t* next_;				// 0x0068
				std::uint32_t flags_;		// 0x0070
				std::uint8_t level_[1];		// 0x0074
				DvarType type_;				// 0x0075
				bool modified_;				// 0x0076
			private:
				char pad_0077[0x0001];		// 0x0077
			public:
				DvarLimits domain_;			// 0x0078
			private:
				char pad_0088[0x0008];		// 0x0088
			} v2_;
		};

		DvarValue& get_current();
		DvarLimits& get_domain();
		DvarValue& get_reset();
		DvarType& get_type();
	};
	ENGINE_ASSERT_SZ(dvar_t, 0x0090);
}
