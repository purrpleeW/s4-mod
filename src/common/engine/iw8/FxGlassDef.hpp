#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/FxCombinedDef.hpp"
#include "engine/iw8/GfxColor.hpp"
#include "engine/iw8/Material.hpp"
#include "engine/iw8/SndAliasLookup.hpp"
#include "engine/iw8/vec2_t.hpp"

namespace iw8 {
	class FxGlassDef {
	public:
		float half_thickness_;						// 0x0000
		vec2_t tex_vecs_[2];						// 0x0004
		GfxColor color_;							// 0x0014
		Material* material_;						// 0x0018
		Material* material_shattered_;				// 0x0020
		const PhysicsAsset* physics_asset_;			// 0x0028
		FxCombinedDef piece_break_effect_;			// 0x0030
		FxCombinedDef shatter_effect_;				// 0x0038
		FxCombinedDef shatter_small_effect_;		// 0x0040
		FxCombinedDef crack_decal_effect_;			// 0x0048
		SndAliasLookup damaged_sound_;				// 0x0050
		SndAliasLookup destroyed_sound_;			// 0x0058
		SndAliasLookup destroyed_quiet_sound_;		// 0x0060
		float high_mip_radius_inv_sq_;				// 0x0068
		float shattered_high_mip_radius_inv_Sq_;	// 0x006C
		int num_crack_rings_;						// 0x0070
		std::uint16_t transient_index_;				// 0x0074
	private:
		char pad_0076[0x0002];						// 0x0076
	};
	ENGINE_ASSERT_SZ(FxGlassDef, 0x0078);
}
