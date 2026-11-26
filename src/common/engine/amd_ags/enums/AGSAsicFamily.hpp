#pragma once
#include "engine/engine_common.hpp"

namespace amd_ags {
	enum class AGSAsicFamily {
		AGSAsicFamily_Unknown,
		AGSAsicFamily_PreGCN,
		AGSAsicFamily_GCN1,
		AGSAsicFamily_GCN2,
		AGSAsicFamily_GCN3,
		AGSAsicFamily_GCN4,
		AGSAsicFamily_Vega,
		AGSAsicFamily_RDNA,
		AGSAsicFamily_RDNA2,
		AGSAsicFamily_RDNA3,
		AGSAsicFamily_RDNA4,
		AGSAsicFamily_Count
	};
}
