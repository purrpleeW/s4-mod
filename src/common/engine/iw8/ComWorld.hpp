#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bitarray.hpp"
#include "engine/iw8/ComChangeListInfo.hpp"

namespace iw8 {
	class ComWorld {
	public:
		const char* name_;										// 0x0000
		int is_in_use_;											// 0x0008
		int use_forward_plus_;									// 0x000C
		std::uint32_t bake_quality_;							// 0x0010
		int proxy_lod_quality_;									// 0x0014
		float tr_vis_radii_[5];									// 0x0018
		float tr_vis_facing_dist_add_;							// 0x002C
		std::uint32_t primary_light_count_;						// 0x0030
	private:
		char pad_0034[0x0004];									// 0x0034
	public:
		ComPrimaryLight* primary_lights_;						// 0x0038
		std::uint32_t scriptable_primary_light_count_;			// 0x0040
		std::uint32_t first_scriptable_primary_light_;			// 0x0044

		union {
			class v1 {
			public:
				ComChangeListInfo change_list_info_;			// 0x0048
				std::uint32_t num_umbra_gates_;					// 0x0058
			private:
				char pad_005C[0x0004];							// 0x005C
			public:
				const char** umbra_gate_names_;					// 0x0060
				bitarray<0x180> umbra_gate_initial_states_;		// 0x0068
			} v1_;

			class v2 {
			public:
				std::uint32_t transient_table_size_;			// 0x0048
			private:
				char pad_004C[0x0004];							// 0x004C
			public:
				std::uint16_t* transient_table_;				// 0x0050
				ComChangeListInfo change_list_info_;			// 0x0058
				std::uint32_t num_umbra_gates_;					// 0x0068
			private:
				char pad_006C[0x0004];							// 0x006C
			public:
				const char** umbra_gate_names_;					// 0x0070
				bitarray<0x180> umbra_gate_initial_states_;		// 0x0078
			} v2_;
		};

		ComChangeListInfo& get_change_list_info();
	};
	ENGINE_ASSERT_SZ(ComWorld, 0x00A8);
}
