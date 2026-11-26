#pragma once
#include "common.hpp"
#include "game/function_types.hpp"

#include <memory/signature_store.hpp>
#include <utils/hashing.hpp>

using namespace utils::hashing::default_hashers::literals;
using namespace utils::hashing::s4::literals;

namespace game {
	using namespace utils::hashing::default_hashers;
	using namespace utils::hashing::s4;

	inline std::uint64_t hash_dvar(const std::string& str) {
		if (identification::game::is_config(identification::game::Beta)) {
			return hash_dvar_beta(str);
		}

		return utils::hashing::s4::hash_dvar(str);
	}

	void init(memory::signature_store& batch);

	bool is_server();

	inline functions::AddRefToValueT* AddRefToValue{};
	inline functions::Com_Error_implT* Com_Error_impl{};
	inline functions::Com_FrontEnd_IsInFrontEndT* Com_FrontEnd_IsInFrontEnd{};
	inline functions::Com_GameInfo_GetGameTypeForInternalNameT* Com_GameInfo_GetGameTypeForInternalName{};
	inline functions::Com_GameInfo_GetMapInfoForLoadNameT* Com_GameInfo_GetMapInfoForLoadName{};
	inline functions::Com_PlayerUtils_ColorizeClanTagT* Com_PlayerUtils_ColorizeClanTag{};
	inline functions::Com_PlayerUtils_ColorizeClanTag_NewT* Com_PlayerUtils_ColorizeClanTag_New{};
	inline functions::Com_SetErrorMessageT* Com_SetErrorMessage{};

	inline functions::DB_FindXAssetHeaderT* DB_FindXAssetHeader{};
	inline functions::DB_GetRawBufferT* DB_GetRawBuffer{};
	inline functions::DB_IsXAssetDefaultT* DB_IsXAssetDefault{};
	inline functions::DB_XAssetExistsT* DB_XAssetExists{};
	inline functions::Dvar_FindMalleableVarT* Dvar_FindMalleableVar{};
	inline functions::Dvar_GetIntSafeT* Dvar_GetIntSafe{};
	inline functions::Dvar_RegisterBoolT* Dvar_RegisterBool{};
	inline functions::Dvar_RegisterIntT* Dvar_RegisterInt{};
	inline functions::Dvar_RegisterVariantT* Dvar_RegisterVariant{};
	inline functions::Dvar_RegisterVec4T* Dvar_RegisterVec4{};
	inline functions::Dvar_ValueToStringT* Dvar_ValueToString{};
	inline functions::GScr_EndLobbyT* GScr_EndLobby{};
	inline functions::GetProtocolVersionT* GetProtocolVersion{};
	inline functions::LiveStorage_IsTimeSyncedT* LiveStorage_IsTimeSynced{};
	inline functions::Live_GetGamePartyT* Live_GetGameParty{};
	inline functions::Live_IsInSystemlinkLobbyT* Live_IsInSystemlinkLobby{};
	inline functions::LUI_Model_CreateModelFromPathT* LUI_Model_CreateModelFromPath{};
	inline functions::LUI_Model_GetGlobalModelT* LUI_Model_GetGlobalModel{};
	inline functions::LUI_Model_SetIntT* LUI_Model_SetInt{};
	inline functions::LUI_Model_SetStringT* LUI_Model_SetString{};
	inline functions::NET_CreateSessionT* NET_CreateSession{};
	inline functions::Party_AreWeServerT* Party_AreWeServer{};
	inline functions::Party_CountAllMembersEvenIfInactiveT* Party_CountAllMembersEvenIfInactive{};
	inline functions::Party_GetGametypeT* Party_GetGametype{};
	inline functions::Party_IsRunningT* Party_IsRunning{};
	inline functions::PartyAtomic_AcceptInviteMPT* PartyAtomic_AcceptInviteMP{};
	inline functions::PartyAtomic_SetupPotentialHostForJoiningT* PartyAtomic_SetupPotentialHostForJoining{};
	inline functions::PartyAtomic_StartJoinT* PartyAtomic_StartJoin{};
	inline functions::PartyHost_PartyRestrictsF2PUsersT* PartyHost_PartyRestrictsF2PUsers{};
	inline functions::RemoveRefToValueT* RemoveRefToValue{};
	inline functions::Scr_ExecThreadT* Scr_ExecThread{};
	inline functions::Scr_FreeThreadT* Scr_FreeThread{};
	inline functions::Scr_GetFunctionHandleT* Scr_GetFunctionHandle{};
	inline functions::Scr_LoadScriptT* Scr_LoadScript{};
	inline functions::ScriptContext_ServerT* ScriptContext_Server{};
	inline functions::StrCypher_DecodeEncryptedStringT* StrCypher_DecodeEncryptedString{};
	inline functions::SL_ConvertToStringT* SL_ConvertToString{};
	inline functions::SL_GetStringT* SL_GetString{};
	inline functions::Sys_MillisecondsT* Sys_Milliseconds{};
	inline functions::UI_ActivisionClanTagAllowedForGamerTagT* UI_ActivisionClanTagAllowedForGamerTag{};
	inline functions::UI_FeederItemTextT* UI_FeederItemText{};
	inline functions::UI_GetMapDisplayNameT* UI_GetMapDisplayName{};
	inline functions::UI_VerifyStringT* UI_VerifyString{};
	inline functions::unk_BNet_IsOnlineT* unk_BNet_IsOnline{};
	inline functions::Voice_ChatRestrictedT* Voice_ChatRestricted{};

	inline s4::CgStatic*(*CgStatic__ms_cgameStaticsArray)[2];
	inline s4::PartyData* g_partyData{};
	inline s4::PartyJoinInfo* g_partyJoinInfo{};
	inline const char* GScr_EndLobby_ConBuf{};
	inline s4::DB_AssetPool* s_assetPools{};
	inline bool* s_luaInFrontend{};
	inline bool* s_UTCSynced{};

	void Cbuf_AddText(const std::string& text);
	void Cbuf_AddText_AutoEncrypt(const std::string& input);
	s4::CgStatic* CgStatic__GetLocalClientStatics(s4::LocalClientNum_t local_client_num);
	const s4::clientInfo_t* Game_GetClientInfo(std::uint32_t client_num);
	void NetadrToSockadr(const s4::netadr_t* a, sockaddr* s);
	const char* Party_GetMapName();
	void R_AddCmdDrawText(const char* text, int max_chars, s4::GfxFont* font, int font_height, float x, float y, float x_scale, float y_scale, float rotation,
		const s4::vec4_t* color);
	const char* SEH_StringEd_GetString(const char* psz_reference);
	void SockadrToNetadr(const sockaddr* s, s4::netadr_t* net_from);
	bool Stringtable_GetAssetNonDefault(const char* file_name, s4::StringTable** table_ptr);
	const char* StringTable_GetColumnValueForRow(s4::StringTable* table, int row, int column);
}
