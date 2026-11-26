#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/SteamID_t.hpp"
#include "engine/steam/enums/EAccountType.hpp"
#include "engine/steam/enums/EUniverse.hpp"

namespace steam {
	class CSteamID {
	public:
		CSteamID() {
			this->steam_id_.comp_.account_id_ = 0;
			this->steam_id_.comp_.account_type_ = EAccountType::k_EAccountTypeInvalid;
			this->steam_id_.comp_.universe_ = EUniverse::k_EUniverseInvalid;
			this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 0);
		}

		CSteamID(std::uint32_t account_id, EUniverse universe, EAccountType account_type) {
			this->set(account_id, universe, account_type);
		}

		CSteamID(std::uint32_t account_id, EChatSteamIDInstanceFlags account_instance, EUniverse universe, EAccountType account_type) {
			this->instanced_set(account_id, account_instance, universe, account_type);
		}

		CSteamID(std::uint64_t steam_id) {
			this->set_from_uint64(steam_id);
		}

		void set(std::uint32_t account_id, EUniverse universe, EAccountType account_type) {
			this->steam_id_.comp_.account_id_ = account_id;
			this->steam_id_.comp_.universe_ = universe;
			this->steam_id_.comp_.account_type_ = account_type;

			if (account_type == EAccountType::k_EAccountTypeClan || account_type == EAccountType::k_EAccountTypeGameServer) {
				this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 0);
			}
			else {
				this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 1 /* k_unSteamUserDefaultInstance */);
			}
		}

		void instanced_set(std::uint32_t account_id, EChatSteamIDInstanceFlags instance, EUniverse universe, EAccountType account_type) {
			this->steam_id_.comp_.account_id_ = account_id;
			this->steam_id_.comp_.universe_ = universe;
			this->steam_id_.comp_.account_type_ = account_type;
			this->steam_id_.comp_.account_instance_ = instance;
		}

		void full_set(std::uint64_t identifier, EUniverse universe, EAccountType account_type) {
			this->steam_id_.comp_.account_id_ = identifier & 0xFFFFFFFF /* k_unSteamAccountIDMask */;
			this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, (identifier >> 32) & 0x000FFFFF /* k_unSteamAccountInstanceMask */);
			this->steam_id_.comp_.universe_ = universe;
			this->steam_id_.comp_.account_type_ = account_type;
		}

		void set_from_uint64(std::uint64_t steam_id) {
			this->steam_id_.all_64_bits_ = steam_id;
		}

		void clear() {
			this->steam_id_.comp_.account_id_ = 0;
			this->steam_id_.comp_.account_type_ = EAccountType::k_EAccountTypeInvalid;
			this->steam_id_.comp_.universe_ = EUniverse::k_EUniverseInvalid;
			this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 0);
		}

		std::uint64_t convert_to_uint64() const {
			return this->steam_id_.all_64_bits_;
		}

		std::uint64_t get_static_account_key() const {
			return (std::uint64_t)((((std::uint64_t)this->steam_id_.comp_.universe_) << 56)
				+ ((std::uint64_t)this->steam_id_.comp_.account_type_ << 52) + this->steam_id_.comp_.account_id_);
		}

		void create_blank_anon_logon(EUniverse universe) {
			this->steam_id_.comp_.account_id_ = 0;
			this->steam_id_.comp_.account_type_ = EAccountType::k_EAccountTypeAnonGameServer;
			this->steam_id_.comp_.universe_ = universe;
			this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 0);
		}

		void create_blank_anon_user_logon(EUniverse universe) {
			this->steam_id_.comp_.account_id_ = 0;
			this->steam_id_.comp_.account_type_ = EAccountType::k_EAccountTypeAnonUser;
			this->steam_id_.comp_.universe_ = universe;
			this->steam_id_.comp_.account_instance_ = VAL_AS(EChatSteamIDInstanceFlags, 0);
		}

		bool is_blank_non_account() const {
			return this->steam_id_.comp_.account_id_ == 0 && this->is_anon_account()
				&& this->steam_id_.comp_.account_instance_ == VAL_AS(EChatSteamIDInstanceFlags, 0);
		}

		bool is_game_server_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeGameServer
				|| this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeAnonGameServer;
		}

		bool is_persistent_game_server_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeGameServer;
		}

		bool is_anon_game_server_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeAnonGameServer;
		}

		bool is_content_server_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeContentServer;
		}

		bool is_clan_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeClan;
		}

		bool is_chat_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeChat;
		}

		bool is_lobby() const {
			return (this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeChat)
				&& ENUM_UNDER(this->steam_id_.comp_.account_instance_ & EChatSteamIDInstanceFlags::k_EChatInstanceFlagLobby);
		}

		bool is_individual_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeIndividual
				|| this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeConsoleUser;
		}

		bool is_anon_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeAnonUser
				|| this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeAnonGameServer;
		}

		bool is_anon_user_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeAnonUser;
		}

		bool is_console_user_account() const {
			return this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeConsoleUser;
		}

		void set_account_id(std::uint32_t account_id) {
			this->steam_id_.comp_.account_id_ = account_id;
		}

		void set_account_instance(EChatSteamIDInstanceFlags instance) {
			this->steam_id_.comp_.account_instance_ = instance;
		}

		AccountID_t get_account_id() const {
			return this->steam_id_.comp_.account_id_;
		}

		EChatSteamIDInstanceFlags get_account_instance() const {
			return this->steam_id_.comp_.account_instance_;
		}

		EAccountType get_account_type() const {
			return this->steam_id_.comp_.account_type_;
		}

		EUniverse get_universe() const {
			return this->steam_id_.comp_.universe_;
		}

		void set_universe(EUniverse universe) {
			this->steam_id_.comp_.universe_ = universe;
		}

		bool is_valid() const {
			if (this->steam_id_.comp_.account_type_ <= EAccountType::k_EAccountTypeInvalid
				|| this->steam_id_.comp_.account_type_ >= EAccountType::k_EAccountTypeMax)
			{
				return false;
			}
			if (this->steam_id_.comp_.universe_ <= EUniverse::k_EUniverseInvalid || this->steam_id_.comp_.universe_ >= EUniverse::k_EUniverseMax) {
				return false;
			}
			if (this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeIndividual && (this->steam_id_.comp_.account_id_ == 0
				|| this->steam_id_.comp_.account_instance_ != VAL_AS(EChatSteamIDInstanceFlags, 1 /* k_unSteamUserDefaultInstance */)))
			{
				return false;
			}
			if (this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeClan && (this->steam_id_.comp_.account_id_ == 0
				|| this->steam_id_.comp_.account_instance_ != VAL_AS(EChatSteamIDInstanceFlags, 0)))
			{
				return false;
			}
			if (this->steam_id_.comp_.account_type_ == EAccountType::k_EAccountTypeGameServer && this->steam_id_.comp_.account_id_ == 0) {
				return false;
			}
			return true;
		}

		explicit CSteamID(const char* steam_id, EUniverse default_universe = EUniverse::k_EUniverseInvalid);
		const char* render() const;
		static const char* render(std::uint64_t steam_id);

		void set_from_string(const char* steam_id, EUniverse default_universe);
		bool set_from_string_strict(const char* steam_id, EUniverse default_universe);

		inline bool operator==(const CSteamID& val) const {
			return this->steam_id_.all_64_bits_ == val.steam_id_.all_64_bits_;
		}

		inline bool operator!=(const CSteamID& val) const {
			return !operator==(val);
		}

		inline bool operator<(const CSteamID& val) const {
			return this->steam_id_.all_64_bits_ < val.steam_id_.all_64_bits_;
		}

		inline bool operator>(const CSteamID& val) const {
			return this->steam_id_.all_64_bits_ > val.steam_id_.all_64_bits_;
		}

		bool is_valid_external_steam_id() const;
	private:
		CSteamID(std::uint32_t);
		CSteamID(std::int32_t);

		SteamID_t steam_id_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(CSteamID, 0x0008);
}
