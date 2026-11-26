#pragma once
#include "engine/engine_common.hpp"

namespace ntl {
	template <typename T, std::size_t Size>
	class fixed_array {
		T data_[Size];
	};
}
