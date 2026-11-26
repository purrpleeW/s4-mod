#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/BgStatic.hpp"
#include "engine/s4/clientInfo_t.hpp"
#include "engine/s4/enums/LocalClientNum_t.hpp"

namespace s4 {
	class CgStatic : public BgStatic {
	public:
		LocalClientNum_t local_client_num_;	// 0x0008
	private:
		char pad_000C[0x0004];				// 0x000C
	public:
		virtual void unk_virtual_00D0() {}
		virtual clientInfo_t* GetClientInfo(const int client_index) const { return nullptr; }
		virtual clientInfo_t* GetClientInfo(const int client_index) { return nullptr; }
		virtual bool GetCharacterAngles(const int entity_index, vec3_t* out_angles) { return false; }
	};
	ENGINE_ASSERT_SZ(CgStatic, 0x0010);
}
