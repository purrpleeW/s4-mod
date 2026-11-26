#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class FenceObject {
	public:
		int controller_index_;				// 0x0008
		bool dependencies_met_;				// 0x000C
		bool is_passing_;					// 0x000D
		bool is_failed_;					// 0x000E
	private:
		char pad_000F[0x0001];				// 0x000F
	public:
		int error_code_;					// 0x0010
	private:
		char pad_0014[0x0004];				// 0x0014
	public:
		std::uint64_t fence_dependencies_;	// 0x0018
		bool ignore_if_guest_;				// 0x0020
	private:
		char pad_0021[0x0007];				// 0x0021
	public:
		void(*callbacks_[4])(int, bool);	// 0x0028
		int failure_draw_time_;				// 0x0048
	private:
		char pad_0034[0x0004];				// 0x004C
	public:
		virtual int get_error_code() = 0;
		virtual int update_is_passing_internal() = 0;
		virtual int update_dependencies(std::uint64_t passing_flags, std::uint64_t failed_flags) = 0;

		bool is_complete() {
			return this->is_passing_ || this->is_failed_;
		}
	};
	ENGINE_ASSERT_SZ(FenceObject, 0x0050);
}
