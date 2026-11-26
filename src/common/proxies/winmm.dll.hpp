// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once
#include "proxies/proxy_base.hpp"

//NOLINTBEGIN

extern "C" {
	BOOL PROXY_API PlaySoundA_ZP(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) {
		static auto func = proxy::get_export<decltype(&PlaySoundA_ZP)>("winmm.dll", "PlaySoundA");
		return func(pszSound, hmod, fdwSound);
	}
	#pragma comment(linker, PROXY_EXPORT(PlaySoundA, 12, 11))

	BOOL PROXY_API PlaySoundW_ZP(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound) {
		static auto func = proxy::get_export<decltype(&PlaySoundW_ZP)>("winmm.dll", "PlaySoundW");
		return func(pszSound, hmod, fdwSound);
	}
	#pragma comment(linker, PROXY_EXPORT(PlaySoundW, 12, 12))

	MMRESULT PROXY_API joyGetDevCapsA_ZP(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc) {
		static auto func = proxy::get_export<decltype(&joyGetDevCapsA_ZP)>("winmm.dll", "joyGetDevCapsA");
		return func(uJoyID, pjc, cbjc);
	}
	#pragma comment(linker, PROXY_EXPORT(joyGetDevCapsA, 12, 22))

	MMRESULT PROXY_API joyGetDevCapsW_ZP(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc) {
		static auto func = proxy::get_export<decltype(&joyGetDevCapsW_ZP)>("winmm.dll", "joyGetDevCapsW");
		return func(uJoyID, pjc, cbjc);
	}
	#pragma comment(linker, PROXY_EXPORT(joyGetDevCapsW, 12, 23))

	UINT PROXY_API joyGetNumDevs_ZP(void) {
		static auto func = proxy::get_export<decltype(&joyGetNumDevs_ZP)>("winmm.dll", "joyGetNumDevs");
		return func();
	}
	#pragma comment(linker, PROXY_EXPORT(joyGetNumDevs, 0, 24))

	MCIERROR PROXY_API mciSendStringA_ZP(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback) {
		static auto func = proxy::get_export<decltype(&mciSendStringA_ZP)>("winmm.dll", "mciSendStringA");
		return func(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
	}
	#pragma comment(linker, PROXY_EXPORT(mciSendStringA, 16, 46))

	MCIERROR PROXY_API mciSendStringW_ZP(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback) {
		static auto func = proxy::get_export<decltype(&mciSendStringW_ZP)>("winmm.dll", "mciSendStringW");
		return func(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
	}
	#pragma comment(linker, PROXY_EXPORT(mciSendStringW, 16, 47))

	MMRESULT PROXY_API midiOutClose_ZP(HMIDIOUT hmo) {
		static auto func = proxy::get_export<decltype(&midiOutClose_ZP)>("winmm.dll", "midiOutClose");
		return func(hmo);
	}
	#pragma comment(linker, PROXY_EXPORT(midiOutClose, 4, 69))

	MMRESULT PROXY_API midiOutGetDevCapsA_ZP(UINT_PTR uDeviceID, LPMIDIOUTCAPSA pmoc, UINT cbmoc) {
		static auto func = proxy::get_export<decltype(&midiOutGetDevCapsA_ZP)>("winmm.dll", "midiOutGetDevCapsA");
		return func(uDeviceID, pmoc, cbmoc);
	}
	#pragma comment(linker, PROXY_EXPORT(midiOutGetDevCapsA, 12, 70))

	MMRESULT PROXY_API midiOutGetDevCapsW_ZP(UINT_PTR uDeviceID, LPMIDIOUTCAPSW pmoc, UINT cbmoc) {
		static auto func = proxy::get_export<decltype(&midiOutGetDevCapsW_ZP)>("winmm.dll", "midiOutGetDevCapsW");
		return func(uDeviceID, pmoc, cbmoc);
	}
	#pragma comment(linker, PROXY_EXPORT(midiOutGetDevCapsW, 12, 71))

	UINT PROXY_API midiOutGetNumDevs_ZP(void) {
		static auto func = proxy::get_export<decltype(&midiOutGetNumDevs_ZP)>("winmm.dll", "midiOutGetNumDevs");
		return func();
	}
	#pragma comment(linker, PROXY_EXPORT(midiOutGetNumDevs, 0, 75))

	MMRESULT PROXY_API mixerClose_ZP(HMIXER hmx) {
		static auto func = proxy::get_export<decltype(&mixerClose_ZP)>("winmm.dll", "mixerClose");
		return func(hmx);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerClose, 4, 93))

	MMRESULT PROXY_API mixerGetControlDetailsA_ZP(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails) {
		static auto func = proxy::get_export<decltype(&mixerGetControlDetailsA_ZP)>("winmm.dll", "mixerGetControlDetailsA");
		return func(hmxobj, pmxcd, fdwDetails);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetControlDetailsA, 12, 94))

	MMRESULT PROXY_API mixerGetControlDetailsW_ZP(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails) {
		static auto func = proxy::get_export<decltype(&mixerGetControlDetailsW_ZP)>("winmm.dll", "mixerGetControlDetailsW");
		return func(hmxobj, pmxcd, fdwDetails);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetControlDetailsW, 12, 95))

	MMRESULT PROXY_API mixerGetDevCapsA_ZP(UINT_PTR uMxId, LPMIXERCAPSA pmxcaps, UINT cbmxcaps) {
		static auto func = proxy::get_export<decltype(&mixerGetDevCapsA_ZP)>("winmm.dll", "mixerGetDevCapsA");
		return func(uMxId, pmxcaps, cbmxcaps);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetDevCapsA, 12, 96))

	MMRESULT PROXY_API mixerGetDevCapsW_ZP(UINT_PTR uMxId, LPMIXERCAPSW pmxcaps, UINT cbmxcaps) {
		static auto func = proxy::get_export<decltype(&mixerGetDevCapsW_ZP)>("winmm.dll", "mixerGetDevCapsW");
		return func(uMxId, pmxcaps, cbmxcaps);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetDevCapsW, 12, 97))

	MMRESULT PROXY_API mixerGetID_ZP(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId) {
		static auto func = proxy::get_export<decltype(&mixerGetID_ZP)>("winmm.dll", "mixerGetID");
		return func(hmxobj, puMxId, fdwId);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetID, 12, 98))

	MMRESULT PROXY_API mixerGetLineControlsA_ZP(HMIXEROBJ hmxobj, LPMIXERLINECONTROLSA pmxlc, DWORD fdwControls) {
		static auto func = proxy::get_export<decltype(&mixerGetLineControlsA_ZP)>("winmm.dll", "mixerGetLineControlsA");
		return func(hmxobj, pmxlc, fdwControls);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetLineControlsA, 12, 99))

	MMRESULT PROXY_API mixerGetLineControlsW_ZP(HMIXEROBJ hmxobj, LPMIXERLINECONTROLSW pmxlc, DWORD fdwControls) {
		static auto func = proxy::get_export<decltype(&mixerGetLineControlsW_ZP)>("winmm.dll", "mixerGetLineControlsW");
		return func(hmxobj, pmxlc, fdwControls);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetLineControlsW, 12, 100))

	MMRESULT PROXY_API mixerGetLineInfoA_ZP(HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo) {
		static auto func = proxy::get_export<decltype(&mixerGetLineInfoA_ZP)>("winmm.dll", "mixerGetLineInfoA");
		return func(hmxobj, pmxl, fdwInfo);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetLineInfoA, 12, 101))

	MMRESULT PROXY_API mixerGetLineInfoW_ZP(HMIXEROBJ hmxobj, LPMIXERLINEW pmxl, DWORD fdwInfo) {
		static auto func = proxy::get_export<decltype(&mixerGetLineInfoW_ZP)>("winmm.dll", "mixerGetLineInfoW");
		return func(hmxobj, pmxl, fdwInfo);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetLineInfoW, 12, 102))

	UINT PROXY_API mixerGetNumDevs_ZP(void) {
		static auto func = proxy::get_export<decltype(&mixerGetNumDevs_ZP)>("winmm.dll", "mixerGetNumDevs");
		return func();
	}
	#pragma comment(linker, PROXY_EXPORT(mixerGetNumDevs, 0, 103))

	MMRESULT PROXY_API mixerOpen_ZP(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen) {
		static auto func = proxy::get_export<decltype(&mixerOpen_ZP)>("winmm.dll", "mixerOpen");
		return func(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerOpen, 20, 105))

	MMRESULT PROXY_API mixerSetControlDetails_ZP(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails) {
		static auto func = proxy::get_export<decltype(&mixerSetControlDetails_ZP)>("winmm.dll", "mixerSetControlDetails");
		return func(hmxobj, pmxcd, fdwDetails);
	}
	#pragma comment(linker, PROXY_EXPORT(mixerSetControlDetails, 12, 106))

	BOOL PROXY_API sndPlaySoundA_ZP(LPCSTR pszSound, UINT fuSound) {
		static auto func = proxy::get_export<decltype(&sndPlaySoundA_ZP)>("winmm.dll", "sndPlaySoundA");
		return func(pszSound, fuSound);
	}
	#pragma comment(linker, PROXY_EXPORT(sndPlaySoundA, 8, 135))

	BOOL PROXY_API sndPlaySoundW_ZP(LPCWSTR pszSound, UINT fuSound) {
		static auto func = proxy::get_export<decltype(&sndPlaySoundW_ZP)>("winmm.dll", "sndPlaySoundW");
		return func(pszSound, fuSound);
	}
	#pragma comment(linker, PROXY_EXPORT(sndPlaySoundW, 8, 136))

	MMRESULT PROXY_API timeBeginPeriod_ZP(UINT uPeriod) {
		static auto func = proxy::get_export<decltype(&timeBeginPeriod_ZP)>("winmm.dll", "timeBeginPeriod");
		return func(uPeriod);
	}
	#pragma comment(linker, PROXY_EXPORT(timeBeginPeriod, 4, 137))

	MMRESULT PROXY_API timeEndPeriod_ZP(UINT uPeriod) {
		static auto func = proxy::get_export<decltype(&timeEndPeriod_ZP)>("winmm.dll", "timeEndPeriod");
		return func(uPeriod);
	}
	#pragma comment(linker, PROXY_EXPORT(timeEndPeriod, 4, 138))

	MMRESULT PROXY_API timeGetDevCaps_ZP(LPTIMECAPS ptc, UINT cbtc) {
		static auto func = proxy::get_export<decltype(&timeGetDevCaps_ZP)>("winmm.dll", "timeGetDevCaps");
		return func(ptc, cbtc);
	}
	#pragma comment(linker, PROXY_EXPORT(timeGetDevCaps, 8, 139))

	DWORD PROXY_API timeGetTime_ZP(void) {
		static auto func = proxy::get_export<decltype(&timeGetTime_ZP)>("winmm.dll", "timeGetTime");
		return func();
	}
	#pragma comment(linker, PROXY_EXPORT(timeGetTime, 0, 141))

	MMRESULT PROXY_API timeKillEvent_ZP(UINT id) {
		static auto func = proxy::get_export<decltype(&timeKillEvent_ZP)>("winmm.dll", "timeKillEvent");
		return func(id);
	}
	#pragma comment(linker, PROXY_EXPORT(timeKillEvent, 4, 142))

	MMRESULT PROXY_API timeSetEvent_ZP(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent) {
		static auto func = proxy::get_export<decltype(&timeSetEvent_ZP)>("winmm.dll", "timeSetEvent");
		return func(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
	}
	#pragma comment(linker, PROXY_EXPORT(timeSetEvent, 20, 143))

	MMRESULT PROXY_API waveInAddBuffer_ZP(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveInAddBuffer_ZP)>("winmm.dll", "waveInAddBuffer");
		return func(hwi, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInAddBuffer, 12, 144))

	MMRESULT PROXY_API waveInClose_ZP(HWAVEIN hwi) {
		static auto func = proxy::get_export<decltype(&waveInClose_ZP)>("winmm.dll", "waveInClose");
		return func(hwi);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInClose, 4, 145))

	MMRESULT PROXY_API waveInGetDevCapsA_ZP(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic) {
		static auto func = proxy::get_export<decltype(&waveInGetDevCapsA_ZP)>("winmm.dll", "waveInGetDevCapsA");
		return func(uDeviceID, pwic, cbwic);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInGetDevCapsA, 12, 146))

	MMRESULT PROXY_API waveInGetDevCapsW_ZP(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic) {
		static auto func = proxy::get_export<decltype(&waveInGetDevCapsW_ZP)>("winmm.dll", "waveInGetDevCapsW");
		return func(uDeviceID, pwic, cbwic);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInGetDevCapsW, 12, 147))

	MMRESULT PROXY_API waveInGetPosition_ZP(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt) {
		static auto func = proxy::get_export<decltype(&waveInGetPosition_ZP)>("winmm.dll", "waveInGetPosition");
		return func(hwi, pmmt, cbmmt);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInGetPosition, 12, 152))

	MMRESULT PROXY_API waveInMessage_ZP(HWAVEIN hwi, UINT uMsg, DWORD_PTR dw1, DWORD_PTR dw2) {
		static auto func = proxy::get_export<decltype(&waveInMessage_ZP)>("winmm.dll", "waveInMessage");
		return func(hwi, uMsg, dw1, dw1);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInMessage, 16, 153))

	MMRESULT PROXY_API waveInOpen_ZP(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen) {
		static auto func = proxy::get_export<decltype(&waveInOpen_ZP)>("winmm.dll", "waveInOpen");
		return func(phwi, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInOpen, 24, 154))

	MMRESULT PROXY_API waveInPrepareHeader_ZP(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveInPrepareHeader_ZP)>("winmm.dll", "waveInPrepareHeader");
		return func(hwi, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInPrepareHeader, 12, 155))

	MMRESULT PROXY_API waveInReset_ZP(HWAVEIN hwi) {
		static auto func = proxy::get_export<decltype(&waveInReset_ZP)>("winmm.dll", "waveInReset");
		return func(hwi);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInReset, 4, 156))

	MMRESULT PROXY_API waveInStart_ZP(HWAVEIN hwi) {
		static auto func = proxy::get_export<decltype(&waveInStart_ZP)>("winmm.dll", "waveInStart");
		return func(hwi);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInStart, 4, 157))

	MMRESULT PROXY_API waveInStop_ZP(HWAVEIN hwi) {
		static auto func = proxy::get_export<decltype(&waveInStop_ZP)>("winmm.dll", "waveInStop");
		return func(hwi);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInStop, 4, 158))

	MMRESULT PROXY_API waveInUnprepareHeader_ZP(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveInUnprepareHeader_ZP)>("winmm.dll", "waveInUnprepareHeader");
		return func(hwi, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveInUnprepareHeader, 12, 159))

	MMRESULT PROXY_API waveOutClose_ZP(HWAVEOUT hwo) {
		static auto func = proxy::get_export<decltype(&waveOutClose_ZP)>("winmm.dll", "waveOutClose");
		return func(hwo);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutClose, 4, 161))

	MMRESULT PROXY_API waveOutGetDevCapsA_ZP(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc) {
		static auto func = proxy::get_export<decltype(&waveOutGetDevCapsA_ZP)>("winmm.dll", "waveOutGetDevCapsA");
		return func(uDeviceID, pwoc, cbwoc);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetDevCapsA, 12, 162))

	MMRESULT PROXY_API waveOutGetDevCapsW_ZP(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc) {
		static auto func = proxy::get_export<decltype(&waveOutGetDevCapsW_ZP)>("winmm.dll", "waveOutGetDevCapsW");
		return func(uDeviceID, pwoc, cbwoc);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetDevCapsW, 12, 163))

	MMRESULT PROXY_API waveOutGetErrorTextA_ZP(MMRESULT mmrError, LPSTR pszText, UINT cchText) {
		static auto func = proxy::get_export<decltype(&waveOutGetErrorTextA_ZP)>("winmm.dll", "waveOutGetErrorTextA");
		return func(mmrError, pszText, cchText);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetErrorTextA, 12, 164))

	MMRESULT PROXY_API waveOutGetErrorTextW_ZP(MMRESULT mmrError, LPWSTR pszText, UINT cchText) {
		static auto func = proxy::get_export<decltype(&waveOutGetErrorTextW_ZP)>("winmm.dll", "waveOutGetErrorTextW");
		return func(mmrError, pszText, cchText);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetErrorTextW, 12, 165))

	UINT PROXY_API waveOutGetNumDevs_ZP(void) {
		static auto func = proxy::get_export<decltype(&waveOutGetNumDevs_ZP)>("winmm.dll", "waveOutGetNumDevs");
		return func();
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetNumDevs, 0, 167))

	MMRESULT PROXY_API waveOutGetPosition_ZP(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt) {
		static auto func = proxy::get_export<decltype(&waveOutGetPosition_ZP)>("winmm.dll", "waveOutGetPosition");
		return func(hwo, pmmt, cbmmt);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetPosition, 12, 170))

	MMRESULT PROXY_API waveOutGetVolume_ZP(HWAVEOUT hwo, LPDWORD pdwVolume) {
		static auto func = proxy::get_export<decltype(&waveOutGetVolume_ZP)>("winmm.dll", "waveOutGetVolume");
		return func(hwo, pdwVolume);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutGetVolume, 8, 171))

	MMRESULT PROXY_API waveOutMessage_ZP(HWAVEIN hwi, UINT uMsg, DWORD_PTR dw1, DWORD_PTR dw2) {
		static auto func = proxy::get_export<decltype(&waveOutMessage_ZP)>("winmm.dll", "waveOutMessage");
		return func(hwi, uMsg, dw1, dw1);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutMessage, 16, 172))

	MMRESULT PROXY_API waveOutOpen_ZP(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen) {
		static auto func = proxy::get_export<decltype(&waveOutOpen_ZP)>("winmm.dll", "waveOutOpen");
		return func(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutOpen, 24, 173))

	MMRESULT PROXY_API waveOutPause_ZP(HWAVEOUT hwo) {
		static auto func = proxy::get_export<decltype(&waveOutPause_ZP)>("winmm.dll", "waveOutPause");
		return func(hwo);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutPause, 4, 174))

	MMRESULT PROXY_API waveOutPrepareHeader_ZP(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveOutPrepareHeader_ZP)>("winmm.dll", "waveOutPrepareHeader");
		return func(hwo, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutPrepareHeader, 12, 175))

	MMRESULT PROXY_API waveOutReset_ZP(HWAVEOUT hwo) {
		static auto func = proxy::get_export<decltype(&waveOutReset_ZP)>("winmm.dll", "waveOutReset");
		return func(hwo);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutReset, 4, 176))

	MMRESULT PROXY_API waveOutRestart_ZP(HWAVEOUT hwo) {
		static auto func = proxy::get_export<decltype(&waveOutRestart_ZP)>("winmm.dll", "waveOutRestart");
		return func(hwo);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutRestart, 4, 177))

	MMRESULT PROXY_API waveOutSetVolume_ZP(HWAVEOUT hwo, DWORD dwVolume) {
		static auto func = proxy::get_export<decltype(&waveOutSetVolume_ZP)>("winmm.dll", "waveOutSetVolume");
		return func(hwo, dwVolume);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutSetVolume, 8, 180))

	MMRESULT PROXY_API waveOutUnprepareHeader_ZP(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveOutUnprepareHeader_ZP)>("winmm.dll", "waveOutUnprepareHeader");
		return func(hwo, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutUnprepareHeader, 12, 181))

	MMRESULT PROXY_API waveOutWrite_ZP(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) {
		static auto func = proxy::get_export<decltype(&waveOutWrite_ZP)>("winmm.dll", "waveOutWrite");
		return func(hwo, pwh, cbwh);
	}
	#pragma comment(linker, PROXY_EXPORT(waveOutWrite, 12, 182))
}

// NOLINTEND