#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/msg_t.hpp"
#include "engine/iw9/enums/svscmd_type.hpp"

namespace iw9 {
	class SvClient {
	public:
		virtual ~SvClient() = 0;
		virtual void SendServerCommand(svscmd_type type, const char* cmd) = 0;
		virtual void SendServerCommandMsg(svscmd_type type, const msg_t* msg) = 0;
		virtual void ResetSnapshotFramesMyChanges() = 0;
		virtual bool IsLocalClient() = 0;
		virtual bool IsTestClient() = 0;
	};
}
