#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/DvarLimits.hpp"
#include "engine/iw9/DvarValue.hpp"
#include "engine/iw9/enums/DvarFlags.hpp"
#include "engine/iw9/enums/DvarType.hpp"

namespace iw9 {
	class dvar_t {
	public:
		XHash hash_;										// 0x0000
		XHash hash_next_;									// 0x0008
		DvarFlags flags_;									// 0x0010
		std::uint8_t level_[1];								// 0x0014
		DvarType type_;										// 0x0015
		bool modified_;										// 0x0016
	private:
		char pad_0017[0x0001];								// 0x0017
	public:
		union {
			class {
			public:
				DvarValue current_;							// 0x0018
				DvarValue current_encrypted_;				// 0x0028
				DvarValue latched_;							// 0x0038
				DvarValue latched_encrypted_;				// 0x0048
				DvarValue reset_;							// 0x0058
				DvarValue reset_encrypted_;					// 0x0068
				DvarLimits domain_;							// 0x0078
			} v1_;

			class {
			public:
				DvarLimits domain_;							// 0x0018
				DvarValue current_;							// 0x0028
				DvarValue current_encrypted_;				// 0x0038
				DvarValue latched_;							// 0x0048
				DvarValue latched_encrypted_;				// 0x0058
				DvarValue reset_;							// 0x0068
				DvarValue reset_encrypted_;					// 0x0078
			} v2_;

			class {
			public:
				DvarLimits domain_;							// 0x0018
				DvarValue current_;							// 0x0028
				DvarValue latched_;							// 0x0038
				DvarValue reset_;							// 0x0048
			} v3_;
		};

		DvarValue& get_current();
		DvarValue& get_reset();
		DvarLimits& get_domain();
	};
	ENGINE_ASSERT_SZ(dvar_t, 0x0088);
}
