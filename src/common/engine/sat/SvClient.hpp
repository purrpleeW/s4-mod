#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/msg_t.hpp"
#include "engine/sat/enums/svscmd_type.hpp"

namespace sat {
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
