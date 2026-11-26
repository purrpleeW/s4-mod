#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/msg_t.hpp"
#include "engine/iw8/netadr_t.hpp"
#include "engine/iw8/PartyActiveClient.hpp"
#include "engine/iw8/PartyData.hpp"
#include "engine/iw8/enums/PartyProfile_EventType.hpp"

namespace iw8 {
	class PartyMessages {
	public:
		const char* string_;													// 0x0000
		void (*func_)(PartyData*, const PartyActiveClient*, netadr_t, msg_t*);	// 0x0008
		PartyProfile_EventType profile_event_type_;								// 0x0010
	private:
		char pad_0014[0x0004];													// 0x0014
	};
	ENGINE_ASSERT_SZ(PartyMessages, 0x0018);
}
