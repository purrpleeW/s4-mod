#pragma once
#include <VMProtectSDK.h>
#include <VirtualizerSDK.h>
#include <StealthCodeArea_Max.h>

#include <obfuscate.h>

#define VMP_KILLSWITCH	TRUE
#define CV_KILLSWITCH	FALSE

#if CLIENT_PROTECTED && !VMP_KILLSWITCH
#	define VMP_PROTECT_START 			VMProtectBegin(__FUNCTION__)
#	define VMP_PROTECT_START_MU 		VMProtectBeginMutation(__FUNCTION__)
#	define VMP_PROTECT_START_ULTRA 		VMProtectBeginUltra(__FUNCTION__)
#	define VMP_PROTECT_END				VMProtectEnd()
#else
#	define VMP_PROTECT_START 			((void)0)
#	define VMP_PROTECT_START_MU 		((void)0)
#	define VMP_PROTECT_START_ULTRA 		((void)0)
#	define VMP_PROTECT_END				((void)0)
#endif

#if CLIENT_PROTECTED && !CV_KILLSWITCH
#	define CV_MUTATE_START 				VIRTUALIZER_MUTATE_ONLY_START
#	define CV_MUTATE_END 				VIRTUALIZER_MUTATE_ONLY_END
#	define CV_VM_TIGER_START  			VIRTUALIZER_TIGER_RED_START 	// 21% Complexity, 94% Speed
#	define CV_VM_TIGER_END  			VIRTUALIZER_TIGER_RED_END
#	define CV_VM_PUMA_START  			VIRTUALIZER_PUMA_BLACK_START   	// 89% Complexity, 3% Speed
#	define CV_VM_PUMA_END  				VIRTUALIZER_PUMA_BLACK_END
#	define CV_VM_SHARK_START  			VIRTUALIZER_SHARK_WHITE_START 	// 80% Complexity, 21% Speed
#	define CV_VM_SHARK_END  			VIRTUALIZER_SHARK_WHITE_END
#	define CV_VM_EAGLE_START  			VIRTUALIZER_EAGLE_WHITE_START	// 92% Complexity, 2% Speed
#	define CV_VM_EAGLE_END  			VIRTUALIZER_EAGLE_WHITE_END
#	define CV_VM_LION_START 			VIRTUALIZER_LION_WHITE_START	// 48% Complexity, 76% Speed
#	define CV_VM_LION_END 				VIRTUALIZER_LION_WHITE_END
#	define CV_VM_DOLPHIN_START  		VIRTUALIZER_DOLPHIN_WHITE_START	// 19% Complexity, 88% Speed
#	define CV_VM_DOLPHIN_END  			VIRTUALIZER_DOLPHIN_WHITE_END
#else
#	define CV_MUTATE_START 				((void)0)
#	define CV_MUTATE_END 				((void)0)
#	define CV_VM_TIGER_START  			((void)0)
#	define CV_VM_TIGER_END  			((void)0)
#	define CV_VM_PUMA_START  			((void)0)
#	define CV_VM_PUMA_END  				((void)0)
#	define CV_VM_SHARK_START  			((void)0)
#	define CV_VM_SHARK_END  			((void)0)
#	define CV_VM_EAGLE_START  			((void)0)
#	define CV_VM_EAGLE_END  			((void)0)
#	define CV_VM_LION_START 			((void)0)
#	define CV_VM_LION_END 				((void)0)
#	define CV_VM_DOLPHIN_START  		((void)0)
#	define CV_VM_DOLPHIN_END  			((void)0)
#endif

#if CLIENT_PROTECTED
#	define STR_OBF(x)	std::string(AY_OBFUSCATE(x))
#else
#	define STR_OBF(x)	std::string(x)
#endif

// double protection in one macro
#define PROTECT_START	VMP_PROTECT_START; CV_VM_TIGER_START
#define PROTECT_END		CV_VM_TIGER_END; VMP_PROTECT_END
