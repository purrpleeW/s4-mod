// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#include "proxies/proxy_base.hpp"

#include "engine/amd_ags/enums/AGSReturnCode.hpp"
#include "engine/amd_ags/AGSContext.hpp"
#include "engine/amd_ags/AGSConfiguration.hpp"
#include "engine/amd_ags/AGSGPUInfo.hpp"

// NOLINTBEGIN
extern "C" {
	amd_ags::AGSReturnCode PROXY_API agsDeInit_ZP(amd_ags::AGSContext* context) {
		static auto func = proxy::get_local_export<decltype(&agsDeInit_ZP)>("amd_ags_x64_og.dll", "agsDeInit");
		return func(context);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDeInit, 4, 1))

	amd_ags::AGSReturnCode PROXY_API agsDriverExtensionsDX11_BeginUAVOverlap_ZP(amd_ags::AGSContext* context) {
		static auto func = proxy::get_local_export<decltype(&agsDriverExtensionsDX11_BeginUAVOverlap_ZP)>("amd_ags_x64_og.dll", "agsDriverExtensionsDX11_BeginUAVOverlap");
		return func(context);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDriverExtensionsDX11_BeginUAVOverlap, 4, 2))

	amd_ags::AGSReturnCode PROXY_API agsDriverExtensionsDX11_DeInit_ZP(amd_ags::AGSContext* context) {
		static auto func = proxy::get_local_export<decltype(&agsDriverExtensionsDX11_DeInit_ZP)>("amd_ags_x64_og.dll", "agsDriverExtensionsDX11_DeInit");
		return func(context);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDriverExtensionsDX11_DeInit, 4, 7))

	amd_ags::AGSReturnCode PROXY_API agsDriverExtensionsDX11_EndUAVOverlap_ZP(amd_ags::AGSContext* context) {
		static auto func = proxy::get_local_export<decltype(&agsDriverExtensionsDX11_EndUAVOverlap_ZP)>("amd_ags_x64_og.dll", "agsDriverExtensionsDX11_EndUAVOverlap");
		return func(context);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDriverExtensionsDX11_EndUAVOverlap, 4, 8))

	amd_ags::AGSReturnCode PROXY_API agsDriverExtensionsDX11_Init_ZP(amd_ags::AGSContext* context, ID3D11Device* device, unsigned int uavSlot, unsigned int* extensionsSupported) {
		static auto func = proxy::get_local_export<decltype(&agsDriverExtensionsDX11_Init_ZP)>("amd_ags_x64_og.dll", "agsDriverExtensionsDX11_Init");
		return func(context, device, uavSlot, extensionsSupported);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDriverExtensionsDX11_Init, 16, 11))

	amd_ags::AGSReturnCode PROXY_API agsDriverExtensionsDX11_SetDepthBounds_ZP(amd_ags::AGSContext* context, bool enabled) {
		static auto func = proxy::get_local_export<decltype(&agsDriverExtensionsDX11_SetDepthBounds_ZP)>("amd_ags_x64_og.dll", "agsDriverExtensionsDX11_SetDepthBounds");
		return func(context, enabled);
	}
#	pragma comment(linker, PROXY_EXPORT(agsDriverExtensionsDX11_SetDepthBounds, 5, 21))

	amd_ags::AGSReturnCode PROXY_API agsGetCrossfireGPUCount_ZP(amd_ags::AGSContext* context, int* numGPUs) {
		static auto func = proxy::get_local_export<decltype(&agsGetCrossfireGPUCount_ZP)>("amd_ags_x64_og.dll", "agsGetCrossfireGPUCount");
		return func(context, numGPUs);
	}
#	pragma comment(linker, PROXY_EXPORT(agsGetCrossfireGPUCount, 8, 27))

	amd_ags::AGSReturnCode PROXY_API agsInit_ZP(amd_ags::AGSContext** context, const amd_ags::AGSConfiguration* config, amd_ags::AGSGPUInfo* gpuInfo) {
		static auto func = proxy::get_local_export<decltype(&agsInit_ZP)>("amd_ags_x64_og.dll", "agsInit");
		return func(context, config, gpuInfo);
	}
#	pragma comment(linker, PROXY_EXPORT(agsInit, 12, 28))
}
// NOLINTEND
