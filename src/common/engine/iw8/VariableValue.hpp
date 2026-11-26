#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/VariableUnion.hpp"
#include "engine/iw8/enums/VariableType.hpp"

namespace iw8 {
	class VariableValue {
	public:
		VariableValue() {
			memset(this, 0, sizeof(VariableValue));
		}

		VariableValue(const VariableUnion& u, const VariableType& type)
			: u_(u)
			, type_(type)
		{}

		VariableUnion u_;		// 0x0000
		VariableType type_;		// 0x0008
	private:
		char pad_0009[0x0007];	// 0x0009
	};
	ENGINE_ASSERT_SZ(VariableValue, 0x0010);
}
