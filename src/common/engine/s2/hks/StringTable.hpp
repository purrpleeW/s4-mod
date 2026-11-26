#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/InternString.hpp"
#include "engine/s2/hks/StringPinner.hpp"

namespace s2::hks {
	class StringTable {
	public:
		InternString** data_;
		hksUint32 count_;
		hksUint32 mask_;
		StringPinner* pinned_strings_;
	};
}
