#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/_XUSER_SIGNIN_STATE.hpp"

namespace iw8 {
	class LocalUserData {
	public:
		_XUSER_SIGNIN_STATE sign_in_state_;			// 0x0000
		char gamertag_[36];							// 0x0004

		union {
			class v1 {
			public:
				std::uint32_t hash_of_gamertag_;	// 0x0028
				char full_gamertag_[64];			// 0x002C
				char platform_gamertag_[36];		// 0x006C
				std::uint64_t xuid_;				// 0x0090
				char xuid_string_[21];				// 0x0098
			private:
				char pad_00AD[0x000B];				// 0x00AD
			public:
				std::uint64_t platform_id_;			// 0x00B8
				char platform_id_string_[21];		// 0x00C0
			private:
				char pad_00D5[0x0013];				// 0x00D5
			} v1_;

			class v2 {
			public:
				int gamertag_suffix_;				// 0x0028
				std::uint32_t hash_of_gamertag_;	// 0x002C
				char full_gamertag_[64];			// 0x0030
				char platform_gamertag_[36];		// 0x0070
			private:
				char pad_0094[0x0004];				// 0x0094
			public:
				std::uint64_t xuid_;				// 0x0098
				char xuid_string_[21];				// 0x00A0
			private:
				char pad_00B5[0x000B];				// 0x00B5
			public:
				std::uint64_t platform_id_;			// 0x00C0
				char platform_id_string_[21];		// 0x00C8
			private:
				char pad_00DD[0x0043];				// 0x00DD
			} v2_;

			class v3 {
			public:
				int gamertag_suffix_;				// 0x0028
				std::uint32_t hash_of_gamertag_;	// 0x002C
				char full_gamertag_[64];			// 0x0030
				char platform_gamertag_[36];		// 0x0070
			private:
				char pad_0094[0x0004];				// 0x0094
			public:
				std::uint64_t xuid_;				// 0x0098
				char xuid_string_[21];				// 0x00A0
			private:
				char pad_00B5[0x0003];				// 0x00B5
			public:
				std::uint64_t platform_id_;			// 0x00B8
				char platform_id_string_[21];		// 0x00C0
			private:
				char pad_00D5[0x0043];				// 0x00D5
			} v3_;
		};

		std::uint32_t& get_hash_of_gamertag();

		char (*get_full_gamertag_ptr())[64];
		char (*get_platform_gamertag_ptr())[36];
		char (*get_xuid_string_ptr())[21];
		char (*get_platform_id_string_ptr())[21];

		std::uint64_t& get_xuid();
		std::uint64_t& get_platform_id();

		void set_xuid(std::uint64_t xuid);
		void set_platform_id(std::uint64_t platform_id);

		// todo: remove when v4 is thrown out
		void assign_username(const char* username, int suffix = 0);
		void assign_xuid(std::uint64_t xuid);
	};
}
