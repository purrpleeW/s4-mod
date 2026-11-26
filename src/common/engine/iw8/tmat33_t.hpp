#include "engine/engine_common.hpp"

namespace iw8 {
	template <typename T>
	union tmat33_t {
		struct {
			T row0_;
			T row1_;
			T row2_;
		};
		T m_[3];
	};
}
