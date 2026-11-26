#include "engine/engine_common.hpp"

namespace iw8 {
	class DBFileHandle {
	public:
		std::uint8_t file_id_[4];		// 0x0000
	private:
		char pad_0004[0x0004];			// 0x0004
	public:
		std::uint64_t d_cache_file_id_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(DBFileHandle, 0x0010);
}
