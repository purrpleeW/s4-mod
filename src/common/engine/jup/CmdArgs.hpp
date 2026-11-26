#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/enums/LocalClientNum_t.hpp"

namespace jup {
	class CmdArgs {
	public:
		int nesting_;							// 0x0000
		LocalClientNum_t local_client_num_[8];	// 0x0004
		int controller_index_[8];				// 0x0024
		int argc_[8];							// 0x0044
	private:
		char pad_0064[0x0004];					// 0x0064
	public:
		const char** argv_[8];					// 0x0068
		char text_pool_[0x4000];				// 0x00A8
		const char* argv_pool_[0x200];			// 0x40A8
		int used_text_pool_[8];					// 0x50A8
		int total_used_argv_pool_;				// 0x50C8
		int total_used_text_pool_;				// 0x50CC
	public:
		const char* arg_v(int arg_index) {
			if (arg_index >= this->argc_[this->nesting_]) {
				return "";
			}
			auto args_cur = this->argv_[this->nesting_];
			if (args_cur == nullptr) {
				return "";
			}
			return args_cur[arg_index];
		}
	};
	ENGINE_ASSERT_SZ(CmdArgs, 0x50D0);
}
