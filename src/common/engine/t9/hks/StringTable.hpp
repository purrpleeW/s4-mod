#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/InternString.hpp"
#include "engine/t9/hks/StringPinner.hpp"

namespace t9::hks {
	class StringTable {
	public:
		InternString** data_;
		hksUint32 count_;
		hksUint32 mask_;
		StringPinner* pinned_strings_;
	};
}
