#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/GenericSignInState.hpp"
#include "engine/iw8/enums/GenericSignOutState.hpp"
#include "engine/iw8/enums/ONLINE_DEPENDENCY_FLAGS.hpp"
#include "engine/iw8/enums/ONLINE_DISCONNECT.hpp"
#include "engine/iw8/enums/StatsSource.hpp"

namespace iw8 {
	class OnlineSystem {
	public:
		virtual void OnRegistered() {}
		virtual void OnUnregistered() {}
		virtual void OnFrontEnd_AllocateSystemsMemory() {}
		virtual void OnFrontEnd_FreeSystemsMemory() {}
		virtual bool Init() { return false; }
		virtual void Uninit() {}
		virtual void Frame() {}
		virtual void ControllerFrame(const int) {}
		virtual void OnSignedIn(const int, GenericSignInState) {}
		virtual void OnUserStatsFetched(const int, StatsSource) {}
		virtual void OnSignedOut(const int, GenericSignOutState) {}
		virtual void OnDisconnect(ONLINE_DISCONNECT) {}
		virtual void OnDWInstantMessage(const int, std::uint64_t, const char*, const char*, std::uint32_t) {}
		virtual void OnDWMail(const int, const char*, const int, bdMailInfo*) {}
		virtual void RegisterLuaFunctions(lua_State*) {}
		virtual void OutputCurrentState(const int) {}
		virtual bool ShowLogOutput() { return false; }

		OnlineSystem* next_system_;					// 0x0008
		ONLINE_DEPENDENCY_FLAGS dependency_flags_;	// 0x0010
		bool initialized_;							// 0x0014
		char name_[0x20];							// 0x0015
		char msg_header_name_[0x08];				// 0x0035
	private:
		char pad_003D[0x0003];						// 0x003D
	};
	ENGINE_ASSERT_SZ(OnlineSystem, 0x0040);
}
