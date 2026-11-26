#pragma once
#include "common_core.hpp"

namespace utils::hashing {
	constexpr std::uint32_t START_DEFAULT_32 = 0x811C9DC5;
	constexpr std::uint32_t IV_DEFAULT_32 = 0x1000193;
	constexpr std::uint32_t SIGN_32 = 0x7FFFFFFF;

	constexpr std::uint64_t START_DEFAULT_64 = 0xCBF29CE484222325;
	constexpr std::uint64_t IV_DEFAULT_64 = 0x100000001B3;
	constexpr std::uint64_t IV_TYPE2_64 = 0x10000000233;
	constexpr std::uint64_t SIGN_64 = 0x7FFFFFFFFFFFFFFF;

	constexpr char validate_char(char c) {
		if (c >= 'A' && c <= 'Z') {
			return 'a' + (c - 'A');
		}
		if (c == '\\') {
			return '/';
		}
		return c;
	}

	constexpr std::uint32_t hash_32a(const std::string& str, std::uint32_t start = START_DEFAULT_32, std::uint32_t iv = IV_DEFAULT_32) {
		std::uint32_t hash = start;
		for (const char* data = str.data(); *data; data++) {
			hash = (hash ^ validate_char(*data)) * iv;
		}
		return hash;
	}

	constexpr std::uint32_t hash_32(const std::string& str, std::uint32_t start = START_DEFAULT_32, std::uint32_t iv = IV_DEFAULT_32) {
		return hash_32a(str, start, iv) & SIGN_32;
	}

	constexpr std::uint64_t hash_64a(const std::string& str, std::uint64_t start = START_DEFAULT_64, std::uint64_t iv = IV_DEFAULT_64) {
		std::uint64_t hash = start;
		for (const char* data = str.data(); *data; data++) {
			hash = (hash ^ validate_char(*data)) * iv;
		}
		return hash;
	}

	constexpr std::uint64_t hash_64(const std::string& str, std::uint64_t start = START_DEFAULT_64, std::uint64_t iv = IV_DEFAULT_64) {
		return hash_64a(str, start, iv) & SIGN_64;
	}

	constexpr std::uint64_t hash_secure_internal(const std::string& str, std::uint64_t start, std::uint64_t iv) {
		if (!start) {
			return 0;
		}
		return hash_64a(str.substr(1), start, iv);
	}

	constexpr std::uint64_t hash_secure(const std::string& str, std::uint64_t start, const std::string& pattern, std::uint64_t iv) {
		if (str.empty()) {
			return 0;
		}
		return hash_secure_internal(str, hash_64a(pattern, (start ^ str.at(0)) * iv, iv), iv);
	}

	constexpr std::uint32_t hash_legacy_treyarch_script(const std::string& str) {
		std::uint32_t hash = 0x4B9ACE2F;
		for (const char* data = str.data(); *data; data++) {
			char c = validate_char(*data);
			hash = ((c + hash) ^ ((c + hash) << 10)) + (((c + hash) ^ ((c + hash) << 10)) >> 6);
		}
		return 0x8001 * ((9 * hash) ^ ((9 * hash) >> 11));
	}

	namespace default_hashers {
		constexpr std::uint32_t hash_32(const std::string& str, std::uint32_t start = START_DEFAULT_32, std::uint32_t iv = IV_DEFAULT_32) {
			return utils::hashing::hash_32(str, start, iv);
		}

		constexpr std::uint32_t hash_32a(const std::string& str, std::uint32_t start = START_DEFAULT_32, std::uint32_t iv = IV_DEFAULT_32) {
			return utils::hashing::hash_32a(str, start, iv);
		}

		constexpr std::uint64_t hash_64(const std::string& str, std::uint64_t start = START_DEFAULT_64, std::uint64_t iv = IV_DEFAULT_64) {
			return utils::hashing::hash_64(str, start, iv);
		}

		constexpr std::uint64_t hash_64a(const std::string& str, std::uint64_t start = START_DEFAULT_64, std::uint64_t iv = IV_DEFAULT_64) {
			return utils::hashing::hash_64a(str, start, iv);
		}

		namespace literals {
			inline constexpr auto operator""_f32(const char* val, std::size_t length) { return hash_32(std::string(val, length)); }
			inline constexpr auto operator""_f32a(const char* val, std::size_t length) { return hash_32a(std::string(val, length)); }
			inline constexpr auto operator""_f64(const char* val, std::size_t length) { return hash_64(std::string(val, length)); }
			inline constexpr auto operator""_f64a(const char* val, std::size_t length) { return hash_64a(std::string(val, length)); }
		}
	}

	namespace t8 {
		constexpr auto hash_dvar(const std::string& str) { return hash_64(str); }
		constexpr auto hash_script(const std::string& str) { return hash_legacy_treyarch_script(str); }

		namespace literals {
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
		}
	}

	namespace t9 {
		inline constexpr auto& hash_dvar = t8::hash_dvar;
		inline constexpr auto& hash_script = t8::hash_script;

		namespace literals {
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
		}
	}

	namespace s4 {
		constexpr auto hash_dvar(const std::string& str) {
			std::uint64_t fixed_start;
			switch (validate_char(str.at(0))) {
			case 'a':	fixed_start = 0xCB916A83C4E2C1FF; break;
			case 'b':	fixed_start = 0x0A7346F232F08A1A; break;
			case 'c':	fixed_start = 0x6F089DD565F89E95; break;
			case 'd':	fixed_start = 0xC95B1F03C4378A70; break;
			case 'e':	fixed_start = 0xB2298F9C0D67123B; break;
			case 'f':	fixed_start = 0xE3F1CD132FA3E9E6; break;
			case 'g':	fixed_start = 0x1117FEA825AF9271; break;
			case 'h':	fixed_start = 0xF5B21BDBA19672AC; break;
			case 'i':	fixed_start = 0xF22A8A5B005B2C47; break;
			case 'j':	fixed_start = 0xBB734DC77DBA2682; break;
			case 'k':	fixed_start = 0xD445A5F1C54FD0FD; break;
			case 'l':	fixed_start = 0x210E062374385478; break;
			case 'm':	fixed_start = 0xBE892DC96DBEA363; break;
			case 'n':	fixed_start = 0xFD0B3DB05C5751CE; break;
			case 'o':	fixed_start = 0x6CE67972894E8F79; break;
			case 'p':	fixed_start = 0x9BB2FBC2F42C3B54; break;
			case 'r':	fixed_start = 0x8A43A0A843922A2A; break;
			case 's':	fixed_start = 0xE3EAD09E517AE525; break;
			case 't':	fixed_start = 0x4C6D2A596F87E580; break;
			case 'u':	fixed_start = 0xA3FBA9FF0FF5F98B; break;
			case 'v':	fixed_start = 0xD567E632548EBBF6; break;
			case 'w':	fixed_start = 0x9640CCE1A1EA0E41; break;
			case 'x':	fixed_start = 0xCD3122D9F8CADC3C; break;
			default:	fixed_start = 0;
			}
			return hash_secure_internal(str, fixed_start, IV_TYPE2_64);
		}

		constexpr auto hash_dvar_beta(const std::string& str) {
			return hash_64a("ouv27kzef3ea681o", hash_64a(str, 0x7BCDAC64BBDF93A4, IV_TYPE2_64), IV_TYPE2_64);
		}

		constexpr std::uint32_t hash_lua_beta(const std::string& str) {
			constexpr std::uint32_t iv = 0xAA40511F;
			return hash_32a(str, 0x41E0A23D, iv) * iv;
		}

		namespace literals {
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
		}
	}

	namespace iw9 {
		constexpr auto hash_asset(const std::string& str) { return hash_64(str, 0x47F5817A5EF961BA); }
		constexpr auto hash_dvar(const std::string& str) { return hash_secure(str, 0xD86A3B09566EBAAC, "q6n-+7=tyytg94_*", IV_TYPE2_64); }
		constexpr auto hash_gamemode(const std::string& str) { return hash_64(str, 0xC7F5817A5EF961BA); }
		constexpr auto hash_script(const std::string& str) { return hash_64a(str, 0x79D6530B0BB9B5D1, IV_TYPE2_64); }

		namespace literals {
			inline constexpr auto operator""_asset(const char* val, std::size_t length) { return hash_asset(std::string(val, length)); }
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_gamemode(const char* val, std::size_t length) { return hash_gamemode(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
		}
	}

	namespace jup {
		inline constexpr auto& hash_asset = iw9::hash_asset;
		inline constexpr auto& hash_dvar = iw9::hash_dvar;
		inline constexpr auto& hash_gamemode = iw9::hash_gamemode;
		inline constexpr auto& hash_script = iw9::hash_script;

		namespace literals {
			inline constexpr auto operator""_asset(const char* val, std::size_t length) { return hash_asset(std::string(val, length)); }
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_gamemode(const char* val, std::size_t length) { return hash_gamemode(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
		}
	}

	namespace t10 {
		inline constexpr auto& hash_asset = iw9::hash_asset;
		inline constexpr auto& hash_dvar = iw9::hash_dvar;

		constexpr auto hash_script(const std::string& str) { return hash_secure(str, 0x1C2F2E3C8A257D07, "zt@f3yp(d[kkd=_@", IV_TYPE2_64); }
		constexpr auto hash_script_sp(const std::string& str) { return hash_64a("zt@f3yp(d[kkd=_@", hash_64a(str, 0x1C2F2E3C8A257D07, IV_TYPE2_64), IV_TYPE2_64); }

		namespace literals {
			inline constexpr auto operator""_asset(const char* val, std::size_t length) { return hash_asset(std::string(val, length)); }
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
			inline constexpr auto operator""_script_sp(const char* val, std::size_t length) { return hash_script_sp(std::string(val, length)); }
		}
	}

	namespace sat {
		inline constexpr auto& hash_asset = iw9::hash_asset;
		inline constexpr auto& hash_dvar = iw9::hash_dvar;
		inline constexpr auto& hash_script = t10::hash_script;

		namespace literals {
			inline constexpr auto operator""_asset(const char* val, std::size_t length) { return hash_asset(std::string(val, length)); }
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
			inline constexpr auto operator""_script(const char* val, std::size_t length) { return hash_script(std::string(val, length)); }
		}
	}

	namespace rex {
		inline constexpr auto& hash_asset = iw9::hash_asset;
		inline constexpr auto& hash_dvar = iw9::hash_dvar;

		namespace literals {
			inline constexpr auto operator""_asset(const char* val, std::size_t length) { return hash_asset(std::string(val, length)); }
			inline constexpr auto operator""_dvar(const char* val, std::size_t length) { return hash_dvar(std::string(val, length)); }
		}
	}
}
