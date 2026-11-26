// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#include "proxies/proxy_base.hpp"

#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

// NOLINTBEGIN

extern "C" {
	discord::EDiscordResult PROXY_API DiscordCreate_ZP(discord::DiscordVersion version, struct DiscordCreateParams* params, struct IDiscordCore** result) {
		return discord::EDiscordResult::DiscordResult_ServiceUnavailable;
	}
#	pragma comment(linker, PROXY_EXPORT(DiscordCreate, 4, 1))
}

// NOLINTEND