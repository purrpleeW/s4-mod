#pragma once
#include "common.hpp"
#include "engine/s4/enums/LocalClientNum_t.hpp"

#include <engine/s4/BattleNetAuth.hpp>
#include <engine/s4/CachedAssets_t.hpp>
#include <engine/s4/CgStatic.hpp>
#include <engine/s4/cmd_function_s.hpp>
#include <engine/s4/CmdArgs.hpp>
#include <engine/s4/ClCommandInfo.hpp>
#include <engine/s4/DB_AssetPool.hpp>
#include <engine/s4/DDLContext.hpp>
#include <engine/s4/dvar_t.hpp>
#include <engine/s4/DvarValue.hpp>
#include <engine/s4/FontGlowStyle.hpp>
#include <engine/s4/gameTypeInfo.hpp>
#include <engine/s4/GfxFont.hpp>
#include <engine/s4/LocalUserData.hpp>
#include <engine/s4/lua_State.hpp>
#include <engine/s4/luaL_Reg.hpp>
#include <engine/s4/luaL_XHashReg.hpp>
#include <engine/s4/mapInfo.hpp>
#include <engine/s4/msg_t.hpp>
#include <engine/s4/netadr_t.hpp>
#include <engine/s4/offset_PlayerStats.hpp>
#include <engine/s4/PartyData.hpp>
#include <engine/s4/ScenarioGroup.hpp>
#include <engine/s4/scr_entref_t.hpp>
#include <engine/s4/StringTable.hpp>
#include <engine/s4/sval_u.hpp>
#include <engine/s4/SvClient.hpp>
#include <engine/s4/VariableValue.hpp>
#include <engine/s4/vec4_t.hpp>
#include <engine/s4/XAssetHeader.hpp>
#include <engine/s4/XSESSION_INFO.hpp>
#include <engine/s4/XUID.hpp>
#include <engine/s4/XArchiveBlocks.hpp>
#include <engine/s4/XZoneMemory.hpp>
#include <engine/s4/enums/CoDClanTagTypes.hpp>
#include <engine/s4/enums/DB_FastFileFailureMode.hpp>
#include <engine/s4/enums/DBMemoryType.hpp>
#include <engine/s4/enums/DLogErrorCode.hpp>
#include <engine/s4/enums/DvarType.hpp>
#include <engine/s4/enums/DWOnlineStatus.hpp>
#include <engine/s4/enums/errorParm_t.hpp>
#include <engine/s4/enums/keyNum_t.hpp>
#include <engine/s4/enums/PartyJoinState.hpp>
#include <engine/s4/enums/PartyJoinType.hpp>
#include <engine/s4/enums/PartyType.hpp>
#include <engine/s4/enums/ScrThreadMode.hpp>
#include <engine/s4/enums/StatsGroup.hpp>
#include <engine/s4/enums/StatsSource.hpp>
#include <engine/s4/enums/sysEventType_t.hpp>
#include <engine/s4/enums/XAssetType.hpp>
#include <engine/s4/SvCommandInfo.hpp>

namespace functions {
	using AddBaseDrawTextCmdT = void*(const char* text, int max_chars, s4::GfxFont* font, s4::Material* font_material, int font_height, float x, float y,
		float x_scale, float y_scale, char tracking, float rotation, const s4::vec4_t* color, int cursor_pos, char cursor, const s4::FontGlowStyle* glow_style,
		bool use_post, int allow_gpad_color, int allow_gpad_any_size, s4::Material* icons_material, bool require_unique_cmd);
	using AddRefToValueT = void(s4::scrContext_t* scr_context, s4::VariableType type, s4::VariableUnion u);
	using BattleNetAuth_BeginAuthT = void(s4::BattleNetAuth* _this);
	using CL_PlayerData_GetDDLBufferT = bool(s4::DDLContext* context, int controller_index, s4::StatsSource stats_source, s4::StatsGroup stats_group);
	using CL_PlayerData_SetClanTagTypeT = void(int controller_index, s4::CoDClanTagTypes clan_tag_type);
	using CL_PlayerData_SetCustomClanTagT = void(int controller_index, const char* clan_tag);
	using CL_PlayerData_GetRankMPT = int(int controller_index);
	using Cmd_AddCommandInternalT = void(const char* cmd_name, void(*function)(), s4::cmd_function_s* alloced_cmd);
	using Cmd_AddClientCommandListT = void(s4::ClCommandInfo* command_list, unsigned int command_list_size);
	using Cmd_AddServerCommandListT = void(s4::SvCommandInfo* command_list, unsigned int command_list_size);
	using Cmd_RemoveServerCommandListT = void(unsigned int command_list_size, s4::SvCommandInfo* command_list);
	using Cmd_RemoveCommandT = void(const char* cmd_name);
	using Cmd_RemoveClientCommandListT = void(s4::ClCommandInfo* command_list, unsigned int command_list_size);
	using Com_Error_implT = void(s4::errorParm_t code, const char* fmt, ...);
	using Com_FrontEnd_IsInFrontEndT = bool();
	using Com_GameInfo_GetGameTypeForInternalNameT = s4::gameTypeInfo*(const char* game_type_name);
	using Com_GameInfo_GetMapInfoForLoadNameT = s4::mapInfo*(const char* map_name);
	using Com_PlayerUtils_ColorizeClanTagT = void(const char* raw_tag, s4::CoDClanTagTypes tag_type, char* out_buffer, std::size_t out_buffer_size, const std::uint8_t post_color);
	using Com_PlayerUtils_ColorizeClanTag_NewT = void(const char* raw_tag, s4::CoDClanTagTypes tag_type, std::uint32_t a3, char* out_buffer, std::size_t out_buffer_size, const std::uint8_t post_color);
	using Com_SetErrorMessageT = void(const char* error_message);
	using DB_AllocXZoneMemoryT = void(std::uint64_t* block_size, const char* file_name, s4::XZoneMemory* zone_mem, s4::XArchiveBlocks* archive_blocks);
	using DB_AllocXZoneMemoryInternalT = void(std::uint64_t* block_size, const char* file_name, s4::XZoneMemory* zone_mem, s4::XArchiveBlocks* archive_blocks,
		s4::DBMemoryType type);
	using DB_FindXAssetHeaderT = s4::XAssetHeader(s4::XAssetType type, const char* given_name, int allow_create_default);
	using DB_GetRawBufferT = void(const s4::RawFile* raw_file, char* buf, int size);
	using DB_IsXAssetDefaultT = int(s4::XAssetType type, const char* name);
	using DB_LoadXFileT = int(const char* zone_name, s4::XZoneMemory* zone_mem, s4::XAssetList* asset_list, std::uint32_t zone_flags, bool was_paused,
		s4::DB_FastFileFailureMode failure_mode, s4::DB_AuthSignature* out_signature);
	using DB_XAssetExistsT = bool(s4::XAssetType type, const char* name);
	using DDL_LocallyObfuscateT = void(s4::DDLContext* ddl_context, std::uint8_t* start, std::uint32_t size, std::uint32_t random_int);
	using DDL_LocallyUnObfuscateT = void(s4::DDLContext* ddl_context, std::uint8_t* start, std::uint32_t size);
	using DLog_RecordErrorEventT = void(s4::DLogErrorCode code, const char* message, const char* stack_trace);
	using DLog_StringT = bool(s4::DLogContext* context, const char* name, const char* value, bool coerce);
	using Dvar_FindMalleableVarT = s4::dvar_t*(std::uint64_t hash);
	using Dvar_GetIntSafeT = int(std::uint64_t hash);
	using Dvar_RegisterBoolT = s4::dvar_t*(std::uint64_t hash, bool value, std::uint32_t flags, const char* description);
	using Dvar_RegisterIntT = s4::dvar_t*(std::uint64_t hash, int value, int min, int max, std::uint32_t flags, const char* description);
	using Dvar_RegisterVariantT = s4::dvar_t*(std::uint64_t hash, std::uint8_t type, std::uint32_t flags, s4::DvarValue* value,
		s4::DvarLimits* domain, const char* description);
	using Dvar_RegisterVec4T = s4::dvar_t*(std::uint64_t hash, float x, float y, float z, float w, float min, float max, std::uint32_t flags,
		const char* description);
	using Dvar_ValueToStringT = const char*(const s4::dvar_t* dvar, s4::DvarValue* value);
	using dwGetLogOnStatusT = s4::DWOnlineStatus(int controller_index);
	using Engine_IsScenarioGroupInstalledT = bool(s4::XHash xhash);
	using FS_StartupT = void(const char* name);
	using G_CmdsMP_ClientCommandT = void(int client_num);
	using G_MainMP_ShutdownGameT = void(bool full_clear);
	using G_MainSP_ShutdownGameT = void(int clear_scripts, bool clear_dvars);
	using G_Spawn_LoadStructsT = void();
	using GamerProfile_LogInProfileT = void(int controller_index);
	using GetProtocolVersionT = int();
	using GScr_EndLobbyT = void(s4::scrContext_t* scr_context);
	using GScr_ExecEntThreadT = std::uint32_t(s4::gentity_s* entity, std::uint32_t handle, std::uint32_t param_count);
	using Live_CanUserPlayOnlineT = bool(int controller_index, std::int32_t* failure_reason);
	using Live_GetGamePartyT = s4::PartyData*();
	using Live_IsInSystemlinkLobbyT = bool();
	using Live_IsUserSignedInToDemonwareT = bool(int controller_index);
	using Live_IsConnectedToDemonwareT = bool(int controller_index);
	using Live_UserSignedInToLiveT = bool(int controller_index, char** disconnect_message);
	using Live_IsDemonwareFailedT = bool(int controller_index, int* status_code);
	using LiveStorage_GetActiveStatsSourceT = s4::StatsSource(int controller_index);
	using LiveStorage_IsTimeSyncedT = bool();
	using LiveStorage_ReadStatsT = void(int controller_index);
	using LiveStorage_StatsInitT = void(int controller_index, bool clear, bool fresh_start, s4::StatsSource stats_source);
	using LocalUserData_GetByControllerT = s4::LocalUserData*(int controller_index);
	using Lua_CoD_PushFunctionToGlobalTableT = void(s4::lua_State* lua_vm, const char* function_name, int(*function_ptr)(s4::lua_State*));
	using lua_insertT = void(s4::lua_State* l, int idx);
	using lua_loadxT = int(s4::lua_State* l, const char*(*reader)(s4::lua_State*, void*, std::uint64_t*), void* data, const char* chunk_name,
		const char* mode);
	using lua_pushbooleanT = void(s4::lua_State* s, int b);
	using lua_pushcclosureT = void(s4::lua_State* s, int(*f)(s4::lua_State*), int n);
	using lua_pushintegerT = void(s4::lua_State* s, int n);
	using lua_pushstringT = void(s4::lua_State* s, const char* str);
	using lua_pushvalueT = void(s4::lua_State* s, int idx);
	using lua_removeT = void(s4::lua_State* s, int idx);
	using lua_settopT = void(s4::lua_State* l, int idx);
	using lua_tolstringT = const char*(s4::lua_State* s, int idx, std::size_t* len);
	using luaL_loadbufferT = int(s4::lua_State* s, const char* buf, std::size_t size, const char* name);
	using luaL_loadfileT = int(s4::lua_State* s, const char* file_name);
	using luaL_openlibT = void(s4::lua_State* s, const char* lib_name, const s4::luaL_Reg* l, std::uint32_t n_up);
	using luaL_registerXHashT = void(s4::lua_State* s, const char* lib_name, const s4::luaL_XHashReg* l);
	using LuaShared_PCallT = bool(s4::lua_State* lua_vm, int n_args, int n_results);
	using LUI_CoD_GetCurrentValidLocalClientT = s4::LocalClientNum_t();
	using LUI_CoD_InitT = void(bool frontend, bool error_recovery);
	using LUI_CoD_ShutdownT = void();
	using LUI_Model_CreateModelFromPathT = std::uint16_t(std::uint16_t parent_node_index, const char* path);
	using LUI_Model_GetGlobalModelT = std::uint16_t();
	using LUI_Model_SetIntT = void(std::uint16_t node_index, int new_value);
	using LUI_Model_SetStringT = void(std::uint16_t node_index, const char* new_value);
	using LUI_OpenMenuT = void(s4::LocalClientNum_t local_client_num, const char* menu_name, bool is_popup, bool is_modal, bool is_exclusive, bool is_immediate);
	using lua_tointeger32T = int(s4::lua_State* l, int idx);
	using NET_CreateSessionT = void(s4::XSESSION_INFO* out_session_info);
	using NET_IsStartedT = bool();
	using Online_Loot_GetItemQuantityT = int(void* _this, int controller_index, const std::uint32_t id);
	using Party_AreWeServerT = bool(const s4::PartyData* party);
	using Party_CountAllMembersEvenIfInactiveT = std::uint32_t(const s4::PartyData* party);
	using Party_GetGametypeT = const char*();
	using Party_IsRunningT = int(const s4::PartyData* party);
	using PartyAtomic_AcceptInviteMPT = void(int controller_index, const s4::XSESSION_INFO* host_info, std::uint64_t lobby_id, const int from_game_invite,
		int from_online_game);
	using PartyAtomic_SetupPotentialHostForJoiningT = bool(int controller_index, s4::XSESSION_INFO* host_info, s4::PartyType party_type,
		const bool is_private_match, s4::PartyJoinInfo* out_party_join_info);
	using PartyAtomic_StartJoinT = void(s4::PartyJoinInfo* party_join_info, s4::PartyData* party, s4::PartyJoinType join_type, s4::PartyJoinState start_state,
		int retry_count);
	using PartyHost_PartyRestrictsF2PUsersT = bool(const s4::PartyData* party);
	using PlatformAuth_FrameT = void();
	using PlatformAuth_GetUsernameT = bool(int controller_index, char* buffer_out, std::size_t buffer_size);
	using PlatformAuth_GetXUIDT = bool(int controller_index, s4::XUID* xuid_out);
	using Playlist_ParsePlaylistsT = void(const char* buffer);
	using ProcessScriptT = void(s4::scrContext_t* context, const char* filename);
	using R_EndFrameT = void();
	using R_RegisterFontT = s4::GfxFont*(const char* name, int pixel_height);
	using R_TextWidthT = int(const char* text, int max_chars, s4::GfxFont* font);
	using RemoveRefToValueT = void(s4::scrContext_t* scr_context, s4::VariableType type, s4::VariableUnion u);
	using Scr_AddClassFieldT = void(s4::scrContext_t* scr_context, std::uint8_t class_num, std::uint32_t name, std::uint32_t canonical_string,
		std::uint32_t offset);
	using Scr_AddStringT = void(s4::scrContext_t* scr_context, const char* value);
	using Scr_BeginLoadScriptsT = void(s4::scrContext_t* scr_context, s4::ScrThreadMode thread_mode);
	using Scr_EndLoadScriptsT = void(s4::scrContext_t* scr_context);
	using Scr_ExecThreadT = std::uint32_t(s4::scrContext_t* scr_context, int handle, std::uint32_t param_count);
	using Scr_FreeThreadT = void(s4::scrContext_t* scr_context, std::uint32_t handle);
	using Scr_GetFunctionHandleT = std::size_t(s4::scrContext_t* scr_context, const char* filename, std::uint32_t handle);
	using Scr_LoadScriptT = std::size_t(s4::scrContext_t* scr_context, const char* filename_ext);
	using Scr_SetThreadPositionT = void(s4::scrContext_t* scr_context, s4::sval_u val);
	using ScriptContext_ServerT = s4::scrContext_t*();
	using SetPresenceT = void(int controller_index);
	using SL_ConvertToStringT = char*(s4::scr_string_t string_value);
	using SL_GetStringT = s4::scr_string_t(const char* str, std::uint32_t user);
	using StrCypher_DecodeEncryptedStringT = const char*(const char* string);
	using SV_CmdsMP_RequestMapRestartT = void(bool load_scripts, bool migrate);
	using Sys_ErrorT = void(const char* error, ...);
	using Sys_MillisecondsT = int();
	using Sys_QueEventT = void(int time, s4::sysEventType_t type, int value, int value2, int ptr_length, void* ptr);
	using UI_ActivisionClanTagAllowedForGamerTagT = bool(int controller_index, const char* clan_tag);
	using UI_FeederItemTextT = const char*(s4::LocalClientNum_t local_client_num, int feeder_id, int index, int column, float* s0, float* t0,
		float* s1, float* t1, s4::Material** material);
	using UI_FillRectPhysicalT = void(float x, float y, float width, float height, s4::vec4_t* color);
	using UI_GetMapDisplayNameT = const char*(const char* psz_map);
	using UI_ValidatePlaylistNumT = void();
	using UI_VerifyStringT = bool(const char* str, const wchar_t* str_orig);
	using unk_BNet_IsOnlineT = bool();
	using unk_FrontEnd_GetZoneNameT = const char*(std::uint8_t mode);
	using unk_OnlineServicesFuncT = bool(void* a1);
	using Voice_ChatRestrictedT = bool();
}
