#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/XNADDR.hpp"
#include "engine/iw9/XSECURITY_INFO.hpp"

namespace iw9 {
	class XSESSION_INFO {
	public:
		XNADDR address_;			// 0x0000
		XSECURITY_INFO security_;	// 0x0054

		bool is_valid_session_id() {
			return this->security_.id_.is_valid();
		}

		void set(const XSECURITY_INFO* security, const XNADDR* addr) {
			this->security_ = *security;
			this->address_ = *addr;
		}
	};
	ENGINE_ASSERT_SZ(XSESSION_INFO, 0x006C);
}
