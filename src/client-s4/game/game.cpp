#include "common.hpp"
#include "game/game.hpp"

#include "component/arxan.hpp"
#include "engine/s4/enums/LocalClientNum_t.hpp"
#include "game/dvars.hpp"
#include "identification/game.hpp"

#include "component/assets.hpp"

#include <utils/flags.hpp>

void game::init(memory::signature_store& batch) {
	batch.add(SETUP_POINTER(Com_Error_impl), "E8 ? ? ? ? 41 8D 46 ? 44 3B F8", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Com_FrontEnd_IsInFrontEnd), "E8 ? ? ? ? 33 D2 0F B6 C8 E8", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Com_GameInfo_GetGameTypeForInternalName), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 8B 35");

	batch.add(SETUP_POINTER(Com_GameInfo_GetMapInfoForLoadName), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 8B 2D");
	
	batch.add(SETUP_POINTER(DB_FindXAssetHeader), "E8 ? ? ? ? 44 8B C5 8D 4D", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(DB_GetRawBuffer), "E8 ? ? ? ? 41 6B 87", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(DB_IsXAssetDefault), "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ?"
		" 48 8B DA 8B F1");

	batch.add(SETUP_POINTER(DB_XAssetExists), "E8 ? ? ? ? 85 C0 0F 95 C0 48 8B 8C 24 ? ? ? ? 48 33 CC E8 ? ? ? ? 48 81 C4 ? ? ? ? C3",
		SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Dvar_FindMalleableVar), "40 53 48 83 EC ? 48 8B D9 F0 FF 05 ? ? ? ? 8B 05 ? ? ? ? 85 C0 74 ? 66 0F 1F 44 00 ? 33 C9 E8 ? ? ? ?"
		" 8B 05 ? ? ? ? 85 C0 75 ? 8B CB");

	batch.add(SETUP_POINTER(Dvar_GetIntSafe), "E8 ? ? ? ? 8B D0 85 C0 75 ? 38 05", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Dvar_RegisterBool), "E8 ? ? ? ? 48 89 05 ? ? ? ? 48 83 C4 ? 5F E9", SETUP_MOD(add(1).rip()));

	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(Dvar_RegisterInt), "E8 ? ? ? ? BA ? ? ? ? 48 89 7C 24 ? 44 8B C2 48 89 05 ? ? ? ? 48 B9 ? ? ? ? ? ? ? ? 89 5C 24 ? 44 8D 4B "
			"? E8 ? ? ? ? BA", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.07.1")) {
		batch.add(SETUP_POINTER(Dvar_RegisterInt), "E8 ? ? ? ? C5 FA 10 0D ? ? ? ? 48 B9 ? ? ? ? ? ? ? ? 48 89 7C 24 ? C4 C1 78 28 D9 C5 E8 57 D2 48 89 05 ? ? ? ? C7 44 24 ? ? ? ? ? E8 ? ? ? ? 48 B9 ? ? ? ? ? ? ? ? 48 89 05",
			SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.10.0")) {
		batch.add(SETUP_POINTER(Dvar_RegisterInt), "E8 ? ? ? ? 48 B9 ? ? ? ? ? ? ? ? 48 89 05 ? ? ? ? 4C 8B CF 41 B8 ? ? ? ? B2 ? E8 ? ? ? ? C5 FA 10 05",
			SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(Dvar_RegisterInt), "E8 ? ? ? ? C5 FA 10 0D ? ? ? ? 48 B9 ? ? ? ? ? ? ? ? 48 89 7C 24 ? C4 C1 78 28 D9 C5 E8 57 D2 C7 44 24",
			SETUP_MOD(add(1).rip()));
	}

	batch.add(SETUP_POINTER(Dvar_RegisterVariant), "E8 ? ? ? ? EB ? 41 B8 ? ? ? ? 48 8B D6 48 8B CF", SETUP_MOD(add(1).rip()));

	if (identification::game::is_less_or_eq("1.10.0")) {
		batch.add(SETUP_POINTER(Dvar_RegisterVec4), "E8 ? ? ? ? C5 F8 28 74 24 ? C5 F8 28 7C 24 ? 48 89 05", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(Dvar_RegisterVec4), "E8 ? ? ? ? C5 FA 10 0D ? ? ? ? C5 7A 10 2D", SETUP_MOD(add(1).rip()));
	}

	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(Dvar_ValueToString), "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B6 41 15 4C 8B DA 83 F8 0B");
	}
	else if (identification::game::is("1.07.1")) {
		batch.add(SETUP_POINTER(Dvar_ValueToString), "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B6 41 ? 4C 8B D2");
	}
	else if (!identification::game::is("1.24.1-ES")) {
		batch.add(SETUP_POINTER(Dvar_ValueToString), "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B6 41 ? 4C 8B C2");
	}
	else {
		batch.add(SETUP_POINTER(Dvar_ValueToString), "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B6 41");
	}

	if (identification::game::is("1.26.1", "1.27.0")) {
		batch.add(SETUP_POINTER(GScr_EndLobby), "40 55 53 57 48 8B EC 48 83 EC ? 48 8B 1D");
	}
	else if (identification::game::is("1.16.0")) {
		batch.add(SETUP_POINTER(GScr_EndLobby), "4C 8B DC 55 53 57 48 8B EC 48 81 EC");
	}
	else if (identification::game::is("1.14.0", "1.10.0", "1.24.1-ES")) {
		batch.add(SETUP_POINTER(GScr_EndLobby), "40 55 53 57 48 8B EC 48 81 EC ? ? ? ? 48 8B 1D");
	}
	else if (identification::game::is("1.07.1")) {
		batch.add(SETUP_POINTER(GScr_EndLobby), "40 53 48 83 EC ? B9 ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 15");
	}
	else if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(GScr_EndLobby), "40 53 48 83 EC ? B9 ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 15 ? ? ? ? 8B D8 8D 0C 02 3B 0D ? ? ? ? 7C ? 50");
	}

	batch.add(SETUP_POINTER(GetProtocolVersion), "E8 ? ? ? ? 3B ? 74 ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(LiveStorage_IsTimeSynced), "E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 03 47", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Live_GetGameParty), "E8 ? ? ? ? 0F B6 D3 48 8B C8 E8 ? ? ? ? E9", SETUP_MOD(add(1).rip()));
 
	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(Live_IsInSystemlinkLobby), "E8 ? ? ? ? 48 8B 7C 24 ? 85 DB", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.07.1")) {
		batch.add(SETUP_POINTER(Live_IsInSystemlinkLobby), "E8 ? ? ? ? 48 8B 7C 24 ? 85 DB", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.10.0")) {
		batch.add(SETUP_POINTER(Live_IsInSystemlinkLobby), "E8 ? ? ? ? 84 C0 75 ? E8 ? ? ? ? 48 8B F8 48 8B CF", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(Live_IsInSystemlinkLobby), "E8 ? ? ? ? 84 C0 75 ? 45 33 C0 8B D3", SETUP_MOD(add(1).rip()));
	}

	batch.add(SETUP_POINTER(LUI_Model_CreateModelFromPath), "E8 ? ? ? ? 33 D2 66 41 89 87", SETUP_MOD(add(1).rip()));
	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(LUI_Model_GetGlobalModel), "E8 ? ? ? ? 33 FF C7 44 24 ? ? ? ? ? C7 44 24", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(LUI_Model_GetGlobalModel), "E8 ? ? ? ? 40 38 2D ? ? ? ? 44 0F B7 C0", SETUP_MOD(add(1).rip()));
	}
	batch.add(SETUP_POINTER(LUI_Model_SetInt), "48 89 74 24 ? 57 48 83 EC ? 0F B7 F9 8B F2");
	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(LUI_Model_SetString), "E8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? BB ? ? ? ? E9", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(LUI_Model_SetString), "40 56 41 56 48 83 EC ? 44 0F B7 F1");
	}
	
	batch.add(SETUP_POINTER(NET_CreateSession), "E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 74 ? C6 43", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Party_AreWeServer), "E8 ? ? ? ? 85 C0 74 ? F6 45", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Party_CountAllMembersEvenIfInactive), "48 89 6C 24 ? 57 48 83 EC ? 33 FF 48 8B E9 39 B9 ? ? ? ? 74");

	batch.add(SETUP_POINTER(Party_GetGametype), "48 83 EC ? E8 ? ? ? ? 80 38");

	batch.add(SETUP_POINTER(Party_IsRunning), "E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 41 B8", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(PartyAtomic_AcceptInviteMP), "E8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8B 93", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(PartyAtomic_SetupPotentialHostForJoining), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ?"
		" 48 8B 74 24 ? 41 8B F8");

	batch.add(SETUP_POINTER(PartyAtomic_StartJoin), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F9 41 8B F0 48 8B EA 48 8B D9 E8");

	batch.add(SETUP_POINTER(PartyHost_PartyRestrictsF2PUsers), "E8 ? ? ? ? 33 D2 88 87", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Scr_ExecThread), "48 83 EC ? 33 C0 45 8B C8");

	batch.add(SETUP_POINTER(Scr_FreeThread), "E8 ? ? ? ? 48 8B 4F ? 48 63 81 ? ? ? ? 48 63 91", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Scr_GetFunctionHandle), "E8 ? ? ? ? 8B E8 85 C0 75 ? 85 F6", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(Scr_LoadScript), "E8 ? ? ? ? 85 C0 75 ? 48 63 4F", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(ScriptContext_Server), "E8 ? ? ? ? 4C 8B C3 41 8B D7 48 8B C8 48 8B E8", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(StrCypher_DecodeEncryptedString), "E8 ? ? ? ? 66 FF 43", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(SL_ConvertToString), "E8 ? ? ? ? 45 33 F6 4C 8B E0", SETUP_MOD(add(1).rip()));

	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(Sys_Milliseconds), "40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 89 83 1C 02 00 00", SETUP_MOD(add(0xA).rip()));
	}
	else if (identification::game::is("1.16.0")) {
		batch.add(SETUP_POINTER(Sys_Milliseconds), "E8 ? ? ? ? 8B CB 03 0D", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(Sys_Milliseconds), "E8 ? ? ? ? 49 8B CE 8B D8 E8 ? ? ? ? 2B C3", SETUP_MOD(add(1).rip()));
	}

	batch.add(SETUP_POINTER(UI_FeederItemText), "E8 ? ? ? ? 48 8B 7C 24 ? 48 8B D8 48 8B 74 24", SETUP_MOD(add(1).rip()));
	batch.add(SETUP_POINTER(UI_GetMapDisplayName), "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4");
	batch.add(SETUP_POINTER(UI_VerifyString), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F B6 01");

	if (identification::game::is("0.02.7-beta-MP")) {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? E8 ? ? ? ? 80 B8 ? ? ? ? ? 0F 84 ? ? ? ? E8 ? ? ? ? 4C 8D 90 ? ? ? ?"
			" 45 8B 4A ? 49 8D 52", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.07.1")) {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 33 F6 48 8D 1D ? ? ? ? 84 C0", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.10.0")) {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? E8 ? ? ? ? 80 B8 ? ? ? ? ? 0F 84 ? ? ? ? E8 ? ? ? ? 4C 8D 90 ? ? ? ?"
			" 45 8B 4A ? 49 8D 52 ? 41 8B C2 4D 8D 42 ? 41 33 C1 8D 48 ? 0F AF C8 8B C1 C1 E8 ? 32 C1 41 32 02 88 44 24", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.16.0")) {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 33 F6 4C 8D 3D ? ? ? ? 84 C0 0F 84", SETUP_MOD(add(1).rip()));
	}
	else if (identification::game::is("1.24.1-ES")) {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 8B 48", SETUP_MOD(add(1).rip()));
	}
	else {
		batch.add(SETUP_POINTER(unk_BNet_IsOnline), "E8 ? ? ? ? 33 F6 4C 8D 25 ? ? ? ? 84 C0 0F 84 ? ? ? ? E8", SETUP_MOD(add(1).rip()));
	}

	batch.add(SETUP_POINTER(Voice_ChatRestricted), "E8 ? ? ? ? 84 C0 75 ? 48 8D 0C 5B", SETUP_MOD(add(1).rip()));

	batch.add(SETUP_POINTER(CgStatic__ms_cgameStaticsArray), "4A 8B ? C1 ? ? ? ? 32 C9", SETUP_MOD(add(4).based_rel()));

	batch.add(SETUP_POINTER(g_partyData), "48 8D 05 ? ? ? ? 4C 63 A0", SETUP_MOD(add(3).rip()));

	batch.add(SETUP_POINTER(g_partyJoinInfo), "48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 83 3D", SETUP_MOD(add(3).rip()));

	batch.add(SETUP_POINTER(GScr_EndLobby_ConBuf), "78 70 61 72 74 79 64 69 73 62 61 6E 64 61 66 74 65 72 72 6F 75 6E 64 0A 00");

	batch.add(SETUP_POINTER(s_assetPools), "FF 44 8B D0 4D 8D 8E ? ? ? ? 41 89 74 C9 10 4D 8D", SETUP_MOD(add(7).based_rel()));

	if (identification::game::is_config(identification::game::Beta)) {
		batch.add(SETUP_POINTER(s_luaInFrontend), "80 3D ? ? ? ? ? 48 63 C1", SETUP_MOD(add(2).rip()));
	}
	else {
		batch.add(SETUP_POINTER(s_luaInFrontend), "0F B6 05 ? ? ? ? 75 ? BF", SETUP_MOD(add(3).rip()));
	}

	batch.add(SETUP_POINTER(s_UTCSynced), "E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 03 47", SETUP_MOD(add(1).rip().add(3).rip()));
}

bool game::is_server() {
	static bool is_server = utils::flags::has_flag("dedicated");
	return is_server;
}

void game::Cbuf_AddText(const std::string& text) {
	std::size_t patch_size = text.size() + 1;
	std::string original_buffer{};
	original_buffer.resize(patch_size);
	std::memcpy(original_buffer.data(), GScr_EndLobby_ConBuf, patch_size);
	utils::hook::copy(PTR_AS(std::uintptr_t, GScr_EndLobby_ConBuf), text.data(), patch_size);
	GScr_EndLobby(nullptr);
	utils::hook::copy(PTR_AS(std::uintptr_t, GScr_EndLobby_ConBuf), original_buffer.data(), patch_size);
}

void game::Cbuf_AddText_AutoEncrypt(const std::string& input) {
	std::vector<std::string> cbuf_commands = {};
	std::vector<std::string> new_cbuf_commands = {};
	std::string cur_cbuf = "";

	for (char ch : input) {
		if (ch == ';') {
			if (cur_cbuf.size() > 0) {
				cbuf_commands.push_back(cur_cbuf);
				cur_cbuf = "";
			}
			continue;
		}
		cur_cbuf += ch;
	}

	if (cur_cbuf.size() > 0) {
		cbuf_commands.push_back(cur_cbuf);
	}

	for (auto& cbuf_command : cbuf_commands) {
		std::string new_cbuf_command = "";
		auto parts = utils::string::split(cbuf_command, ' ');

		for (const auto& [hash, dvar] : dvars::dvar_map) {
			auto name = utils::string::to_lower(dvar.name);
			if (name == utils::string::to_lower(parts[0])) {
				new_cbuf_command = std::format("#x3{:016X}u64", dvar.hash);
				break;
			}
		}

		if (new_cbuf_command.empty()) {
			new_cbuf_command = parts[0];
		}

		for (std::size_t x = 1; x < parts.size(); x++) {
			new_cbuf_command += " " + parts[x];
		}

		new_cbuf_commands.push_back(new_cbuf_command);
	}

	std::string fixed_fixed_input = utils::string::join(new_cbuf_commands, ";");
	game::Cbuf_AddText(std::format("{} \n", fixed_fixed_input));
}

s4::CgStatic* game::CgStatic__GetLocalClientStatics(s4::LocalClientNum_t local_client_num) {
	return (*CgStatic__ms_cgameStaticsArray)[ENUM_UNDER(local_client_num)];
}

const s4::clientInfo_t* game::Game_GetClientInfo(std::uint32_t client_num) {
	return CgStatic__GetLocalClientStatics(s4::LocalClientNum_t::LOCAL_CLIENT_0 /* LUI_CoD_GetCurrentValidLocalClient() */)->GetClientInfo(client_num);
}

void game::NetadrToSockadr(const s4::netadr_t* a, sockaddr* s) {
	if (a->type_ != s4::netadrtype_t::NA_BROADCAST && a->type_ != s4::netadrtype_t::NA_IP && a->type_ != s4::netadrtype_t::NA_RAW) {
		return;
	}

	s->sa_family = AF_INET;
	DEREF_PTR_AS(std::uint16_t, &s->sa_data[0]) = htons(a->port_);

	if (a->type_ == s4::netadrtype_t::NA_BROADCAST) {
		DEREF_PTR_AS(std::uint32_t, &s->sa_data[2]) = 0xFFFFFFFF;	// -1 / 255.255.255.255
		return;
	}

	for (auto x = 0; x < 4; x++) {
		s->sa_data[2 + x] = a->ip_[x];
	}
}

const char* game::Party_GetMapName() {
	auto ui_mapname = dvars::get_dvar_by_name("ui_mapname");
	return ui_mapname ? ui_mapname->get_current().string_ : "";
}



const char* game::SEH_StringEd_GetString(const char* psz_reference) {
	if (psz_reference == nullptr || Dvar_FindMalleableVar == nullptr) {
		return "<unknown>";
	}

	static s4::dvar_t* loc_translate = dvars::get_dvar_by_name("loc_translate");
	if (loc_translate == nullptr || !loc_translate->get_current().enabled_ || *psz_reference == NULL) {
		return psz_reference;
	}

	s4::LocalizeEntry* entry = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_LOCALIZE, psz_reference, 0).localize_;
	if (entry == nullptr) {
		return psz_reference;
	}

	return entry->value_ ? entry->value_ : psz_reference;
}

void game::SockadrToNetadr(const sockaddr* s, s4::netadr_t* net_from) {
	if (s->sa_family != 2) {
		return;
	}

	net_from->type_ = s4::netadrtype_t::NA_IP;
	for (auto x = 0; x < 4; x++) {
		net_from->ip_[x] = s->sa_data[2 + x];
	}
	net_from->port_ = ntohs(DEREF_PTR_AS(const std::uint16_t, &s->sa_data[0]));
}

bool game::Stringtable_GetAssetNonDefault(const char* file_name, s4::StringTable** table_ptr) {
	*table_ptr = assets::find_x_asset_header(s4::XAssetType::ASSET_TYPE_STRINGTABLE, file_name, 0).string_table_;
	return *table_ptr != nullptr;
}

const char* game::StringTable_GetColumnValueForRow(s4::StringTable* table, int row, int column) {
	auto column_count = table->column_count_;
	auto row_count = table->row_count_;

	if (row < 0 || row >= row_count || column < 0 || column >= column_count) {
		return "";
	}

	auto cell_index = table->cell_indices_[column + row * column_count];
	if (cell_index >= table->unique_cell_count_) {
		return "";
	}

	// return utils::string::va("[%d~%d->%d/%llx]", row, column, cell_index, PTR_AS(std::uintptr_t, table->strings_[cell_index]));
	return table->strings_[cell_index];
}
