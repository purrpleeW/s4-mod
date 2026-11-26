#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdDTLSAssociationTelemetry.hpp"
#include "engine/iw8/NetAddress.hpp"
#include "engine/iw8/NetSession.hpp"

namespace iw8 {
	class NetEndpoint {
	public:
		NetAddress address_;					// 0x0000
		NetConnection* connections_[2][3][3];	// 0x0010
		NetSession* session_;					// 0x00A0
		int reference_count_;					// 0x00A8
		int connection_count_;					// 0x00AC
		int last_recv_;							// 0x00B0
		int last_sent_;							// 0x00B4
		bool graveyarded_;						// 0x00B8
	private:
		char pad_00B9[0x0007];					// 0x00B9
	public:
		bdDTLSAssociationTelemetry telemetry_;	// 0x00C0
	};
	ENGINE_ASSERT_SZ(NetEndpoint, 0x0190);
}
