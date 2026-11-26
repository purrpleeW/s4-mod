#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/netadr_t.hpp"
#include "engine/iw8/enums/netsrc_t.hpp"

namespace iw8 {
	class NetConnection {
	public:
		enum class Type {
			TYPE_NONE = 0,
			TYPE_PARTY = 1,
			TYPE_LOBBY = 2,
			TYPE_JOIN = 3,
			TYPE_FRONTEND = 4,
			TYPE_GAME = 5
		};

		NetEndpoint* endpoint_;
		NetChannel* channel_;
		netsrc_t local_id_;
		netsrc_t remote_id_;
		Type type_;

		bool compare_addr(const netadr_t* addr) {
			return DEREF_PTR_AS(int, this->endpoint_ /* this->endpoint_->index_ */) == addr->addr_handle_index_;
		}
	};
	ENGINE_ASSERT_SZ(NetConnection, 0x0020);
}
