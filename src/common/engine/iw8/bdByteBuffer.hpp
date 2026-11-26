#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdReferencable.hpp"

namespace iw8 {
	class bdByteBuffer : public bdReferencable {
	public:
		virtual void allocateBuffer() {}
		virtual bool expand(std::uint32_t) { return false; }
		virtual void resizeToFit(std::uint32_t) {}

		std::uint32_t size_;		// 0x0010
	private:
		char pad_0014[0x0004];		// 0x0014
	public:
		std::uint32_t* data_;		// 0x0018
		std::uint32_t* read_ptr_;	// 0x0020
		std::uint32_t* write_ptr_;	// 0x0028
		bool type_checked_;			// 0x0030
		bool type_checked_copy_;	// 0x0031
		bool allocated_data_;		// 0x0032
	private:
		char pad_0033[0x0005];		// 0x0033
	};
	ENGINE_ASSERT_SZ(bdByteBuffer, 0x0038);
}
