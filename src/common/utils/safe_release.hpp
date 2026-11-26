#pragma once
#include "common_core.hpp"

namespace utils {
	template <typename F>
	class safe_release final {
		F&& fn_;
	public:
		explicit safe_release(F&& f) : fn_(std::forward<F>(f)) {}
		safe_release(const safe_release&) = delete;
		safe_release& operator=(const safe_release&) = delete;
		safe_release(safe_release&&) = delete;

		~safe_release() { fn_(); }
	};
}