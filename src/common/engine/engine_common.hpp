#pragma once
#include "common_core.hpp"

#define ENGINE_ASSERT_SZ(cls, sz) static_assert(sizeof(cls) == sz, #cls " is not " #sz " bytes in size.")

namespace engine_global {
	template <typename N>
	class base_class {
	public:
		static inline N* create(void* ptr) {
			return PTR_AS(N*, ptr);
		}

		template <typename T>
		T* get(std::uintptr_t offset) {
			return PTR_AS(T*, PTR_AS(std::uintptr_t, this) + offset);
		}
	};
}

namespace discord {
	using DiscordVersion = int;
}

namespace s4 {
	using BgAnimStatic = void;
	using BgPlayerEvents = void;
	using characterAnimState_t = void;
	using characterInfo_t = void;
	using DB_AuthSignature = void;
	using DLogContext = void;
	using DObj = void;
	using DObjModel = void;
	using DObjPartBits = void;
	using entityState_t = void;
	using gclient_s = void;
	using gentity_s = void;
	using Material = void;
	using PartyJoinInfo = void;
	using scr_block_t = void;
	using scrContext_t = void;
	using Weapon = void;
	using XHash = std::uint64_t;
	using XAnimTree = void;
	using XAssetList = void;
	using XModel = void;
	using XZoneTemporaryLoadData = void;
}

namespace uwp {
	using XStoreContextHandle = class XStoreContext*;
	using XStoreLicenseHandle = class XStoreLicense*;
	using XStoreProductQueryHandle = class XStoreProductQuery*;
	using XTaskQueueHandle = class XTaskQueueObject*;
	using XUserHandle = class XUser*;

	using XStoreGameLicenseChangedCallback = void(void* context);
	using XStorePackageLicenseLostCallback = void(void* context);
}
