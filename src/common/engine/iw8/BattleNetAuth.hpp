#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/CoDAuthState.hpp"
#include "engine/iw8/BattleNetString.hpp"

namespace iw8 {
	class BattleNetAuth {
	public:
		CoDAuthState state_;			// 0x0000
	private:
		char pad_0004[0x00EC];			// 0x0004
	public:
		BattleNetString battle_tag_;	// 0x00F0
	private:
		char pad_0100[0x0018];			// 0x0100
	public:
		BattleNetString client_id_;		// 0x0118
	private:
		char pad_0128[0x0040];			// 0x0128
	public:
		BattleNetString email_;			// 0x0168
	private:
		char pad_0178[0x0050];			// 0x0178
	public:
		int error_code_;				// 0x01C8
	private:
		char pad_01CC[0x0104];			// 0x01CC
	public:
		bool finished_auth_;			// 0x02D0
		bool live_igr_;					// 0x02D1
		bool live_igr_paid_;			// 0x02D2
	private:
		char pad_02D3[0x0021];			// 0x02D3
	public:
		int var3_;						// 0x02F4
		char var4_;						// 0x02F8
	private:
		char pad_02F9[0x0003];			// 0x02F9
	public:
		int var5_;						// 0x02FC
	private:
		char pad_0300[0x0014];			// 0x0300
	public:
		bool disconnected_;				// 0x0314
	private:
		char pad_0315[0x0013];			// 0x0315
	};
	ENGINE_ASSERT_SZ(BattleNetAuth, 0x0328);
}
