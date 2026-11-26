#pragma once
#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <atlsafe.h>
#include <conio.h>
#include <corecrt_io.h>
#include <csetjmp>
#include <d3d11.h>
#include <d3d12.h>
#include <d3d9.h>
#include <DbgHelp.h>
#include <direct.h>
#include <dwmapi.h>
#include <dxgi1_6.h>
#include <ExDisp.h>
#include <fcntl.h>
#include <iphlpapi.h>
#include <mmeapi.h>
#include <Mmsystem.h>
#include <MsHtmHst.h>
#include <MsHTML.h>
#include <Psapi.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <ShlObj.h>
#include <TlHelp32.h>
#include <urlmon.h>
#include <VersionHelpers.h>
#include <Windows.h>
#include <wincrypt.h>
#include <WinSock2.h>
#include <winternl.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "ws2_32.lib")

// min and max is required by gdi, therefore NOMINMAX won't work
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#	define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifdef max
#	undef max
#endif
#ifdef min
#	undef min
#endif
#undef GetObject

#include <cassert>
#include <cctype>
#include <climits>
#include <cstring>

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <sstream>
#include <stacktrace>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "logger/ansi.hpp"

using namespace std::literals;

#define VT_GET(ptr, idx) (*(void***)(ptr))[idx]
#define PTR_AS(type, ptr) reinterpret_cast<type>((ptr))
#define VAL_AS(type, val) static_cast<type>((val))
#define DEREF_PTR_AS(type, ptr) *PTR_AS(type*, ptr)
#define ENUM_UNDER(val) static_cast<std::underlying_type_t<decltype(val)>>(val)
#define CLASS_ASSERT_SZ(cls, sz) static_assert(sizeof(cls) == sz, #cls " is not " #sz " bytes in size.")
#define CLASS_VALUE_AT_PTR(cls, off, t) PTR_AS(t*, PTR_AS(std::uintptr_t, cls) + off)
#define CLASS_VALUE_AT(cls, off, t) DEREF_PTR_AS(t, PTR_AS(std::uintptr_t, cls) + off)
#define RSC_VAL(arg) PTR_AS(std::uintptr_t, arg)
#if defined(_WIN64)
#	define CLASS_ASSERT_SZ_64(cls, sz) CLASS_ASSERT_SZ(cls, sz)
#	define CLASS_ASSERT_SZ_86(cls, sz) ;
#	define FMT_PTR "{:016X}"
#else
#	define CLASS_ASSERT_SZ_64(cls, sz) ;
#	define CLASS_ASSERT_SZ_86(cls, sz) CLASS_ASSERT_SZ(cls, sz)
#	define FMT_PTR "{:08X}"
#endif

#if !defined(CLIENT_PRERELEASE) || !CLIENT_PRERELEASE
#	define ZPDS(str) ("<dev string L" ANSI_STRINGIFY_LAYERED(__LINE__) ">")
#	define ZPDS2(str) __FUNCTION__ "#" ANSI_STRINGIFY_LAYERED(__LINE__)
#else
#	define ZPDS(str) (str)
#	define ZPDS2(str) (str)
#endif

#define SETUP_POINTER(name) ZPDS2(#name), (void**)&name
#define SETUP_POINTER_V(name) ZPDS2(#name), (std::vector<void*>*)&name
#define SETUP_MOD(mod) [](memory::scanned_result<void> r) { return r.mod; }

#define DECLARE_BITWISE_OPS(enum_class) \
	constexpr enum_class operator|(enum_class a, enum_class b) { \
		return static_cast<enum_class>(ENUM_UNDER(a) | ENUM_UNDER(b)); \
	} \
	constexpr enum_class& operator|=(enum_class& a, enum_class b) { \
		a = a | b; \
		return a; \
	} \
	constexpr enum_class operator&(enum_class a, enum_class b) { \
		return static_cast<enum_class>(ENUM_UNDER(a) & ENUM_UNDER(b)); \
	} \
	constexpr enum_class& operator&=(enum_class& a, enum_class b) { \
		a = a & b; \
		return a; \
	}

#define PROTECT_VOID(var_name) \
	static bool var_name = false; \
	if (var_name) { \
		return; \
	} \
	var_name = true;

enum class pipe_message_type : std::uint8_t {
	set_console_title = 0,
	print = 1,
	clear = 2
};

std::uintptr_t operator""_b(unsigned long long int val);
std::string and_rel(std::uintptr_t address);

inline HMODULE client_module = nullptr;

/** Comment Colors - Please actually use these **/
//! Urgent, Important and perchance rants
/// Doxygen doc comments, useful for function docs
//@ TODOs, Stuff to remember for later
//? Question all your life choices
//~ Rants, personal stuff, etc etc
//& Explanations and links to our resources
//$ Links to external stuff and also misc highlights
//§ Deprecated stuff, only there because yeah forgot