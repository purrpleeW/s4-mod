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
	using DiscordBranch = char[4096];
	using DiscordClientId = std::int64_t;
	using DiscordDateTime = char[64];
	using DiscordLobbyId = std::int64_t;
	using DiscordLobbySecret = char[128];
	using DiscordLocale = char[128];
	using DiscordMetadataKey = char[256];
	using DiscordMetadataValue = char[4096];
	using DiscordNetworkChannelId = std::uint8_t;
	using DiscordNetworkPeerId = std::uint64_t;
	using DiscordPath = char[4096];
	using DiscordSnowflake = std::int64_t;
	using DiscordTimestamp = std::int64_t;
	using DiscordUserId = std::int64_t;
	using DiscordVersion = int;
	using IDiscordApplicationEvents = void*;
	using IDiscordCoreEvents = void*;
	using IDiscordImageEvents = void*;
	using IDiscordStorageEvents = void*;
}

namespace iw4s {
	using Material = void;
}

namespace iw6s {
	using Material = void;
	using snd_alias_list_t = void;
}

namespace iw8 {
	class bdJSONArray;
	class bdJSONObject;
	class bdJSONValue;
	class lua_State;
	class NetChannel;
	class NetConnection;
	class NetEndpoint;

	using AccessoryDef = void;
	using AddonMapEnts = void;
	using AnimationClass = void;
	using Animset = void;
	using ASM = void;
	using BCIns = std::uint32_t;
	using BCPos = std::uint32_t;
	using BCLine = int;
	using bdMailInfo = int;
	using BehaviorTree = void;
	using BlendSpace2DDef = void;
	using BulletPenetration = void;
	using CameraDef = void;
	using CarryObjectDef = void;
	using CinematicMotionDef = void;
	using ClConnection = void;
	using ClConnectionMP = void;
	using ClientCharacter = void;
	using clipMap_t = void;
	using ClothAsset = void;
	using CollisionTile = void;
	using ComPrimaryLight = void;
	using ComputeShader = void;
	using CoverSelector = void;
	using DB_AssetEntryPool = void;
	using DB_AssetEntryTable = void;
	using DB_FFHeader = void;
	using DDLFile = void;
	using DLogContext = void;
	using DLogSchema = void;
	using DynEntityList = void;
	using EnemySelector = void;
	using EquipmentSoundTable = void;
	using ExecutionDef = void;
	using FootstepVFX = void;
	using FuncState = void;
	using FxImpactTable = void;
	using FxParticleSimAnimation = void;
	using FxWorld = void;
	using FxWorldTransientZone = void;
	using GamerSettingState = void;
	using gclient_s = void;
	using GCSize = std::size_t;
	using gentity_s = void;
	using Gesture = void;
	using GfxDecalVolumeMask = void;
	using GfxFogSpline = void;
	using GfxGradingClut = void;
	using GfxIESProfile = void;
	using GfxLightDef = void;
	using GfxWorld = void;
	using GfxWorldTransientZone = void;
	using GlassWorld = void;
	using HudOutlineDef = void;
	using KeyValuePairs = void;
	using LaserDef = void;
	using LeaderboardDef = void;
	using LexChar = int;
	using LexToken = int;
	using LocDmgTable = void;
	using lua_Reader = const char*(*)(lua_State* l, void* ud, std::size_t* sz);
	using LUIGlobalPackage = void;
	using LUIPlayercardPackage = void;
	using MapEdgeList = void;
	using MapEnts = void;
	using MayhemData = void;
	using MSize = std::uint32_t;
	using NativeScriptPatchFile = void;
	using NavMeshData = void;
	using Online_MetPlayer = void;
	using OnlineChallengesManager = void;
	using OnlineChat = void;
	using OnlineChatPool = void;
	using OnlineMatchmakerOmniscient = void;
	using OnlineUserLists = void;
	using ParticleSystemDef = void;
	using PartyDisconnectReason = std::uint8_t;
	using PathData = void;
	using PlayerAnimScript = void;
	using PlayerProfileData = void;
	using playerState_s = void;
	using PhysicsAsset = void;
	using PhysicsDebugData = void;
	using PhysicsFXPipeline = void;
	using PhysicsFXShape = void;
	using PhysicsLibrary = void;
	using PhysicsSFXEventAsset = void;
	using PhysicsVFXEventAsset = void;
	using ReticleDef = void;
	using RumbleGraph = void;
	using RumbleInfo = void;
	using scr_block_t = void;
	using ScriptableDef = void;
	using ScriptBundle = void;
	using ScriptDebugData = void;
	using SndAlias = void;
	using SndBankResident = void;
	using SndBankTransient = void;
	using SndGlobals = void;
	using SoundBankListDef = void;
	using SpaceshipTargetDef = void;
	using StreamingInfo = void;
	using StreamKey = void;
	using StreamTreeOverride = void;
	using StTerrain = void;
	using SuitAnimPackage = void;
	using SuitDef = void;
	using SurfaceFxTable = void;
	using TacticalGraphData = void;
	using TracerDef = void;
	using UserList = void;
	using VectorField = void;
	using VehicleDef = void;
	using VirtualLeaderboardDef = void;
	using WeaponAnimPackage = void;
	using WeaponAttachment = void;
	using WeaponCompleteDef = void;
	using WeaponSFXPackage = void;
	using WeaponVFXPackage = void;
	using XAnimCurve = void;
	using XAnimDynamicBones = void;
	using XAnimParts = void;
	using XAnimProceduralBones = void;
	using XCam = void;
	using XCompositeModelDef = void;
	using XG_RESOURCE_LAYOUT = void;
	using XModel = void;
	using XModelDetailCollision = void;
	using XModelSurfs = void;
	using XZoneTemporaryLoadData = void;
}

namespace iw9 {
	using gclient_s = void;
	using gentity_s = void;
	using LootMasterType = void;
	using Material = void;
	using PartyJoinInfo = void;
	using scrContext_t = void;
	using XHash = std::uint64_t;
}

namespace jup {
	using gclient_s = void;
	using gentity_s = void;
	using netadr_t = void;
	using PartyJoinInfo = void;
	using scrContext_t = void;
	using XHash = std::uint64_t;
}

namespace s2 {
	class _jmp_buf;
	class lua_Debug;
	class lua_State;

	using hks_debug_map = int(*)(const char*, int);
	using hksBool = int;
	using hksByte = std::uint8_t;
	using hksBytecodeInstruction = std::uint32_t;
	using hksChar = char;
	using HksEmergencyGCFailFunc = void(*)(lua_State*, std::size_t);
	using hksInt32 = int;
	using hksInt64 = std::int64_t;
	using HksLogFunc = void(*)(lua_State*, const char*, ...);
	using HksNativeValueAsInt = std::uint32_t;
	using HksNumber = float;
	using hksShort16 = short;
	using hksSize = std::size_t;
	using hksUint32 = std::uint32_t;
	using hksUint64 = std::uint64_t;
	using hksUshort16 = std::uint16_t;
	using jmp_buf = _jmp_buf[1];
	using lua_Alloc = void*(*)(void*, void*, std::size_t, std::size_t);
	using lua_CFunction = hksInt32(*)(lua_State*);
	using lua_Hook = void(*)(lua_State*, lua_Debug*);

	namespace hks {
		class CallSite;
		class cclosure;
		class Debugger;
		class HashTable;
		class HksClosure;
		class InternString;
		class StructInst;
		class UserData;

		using ErrorHandler = CallSite*;
		using HksGcCost = int;
	}
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

namespace sat {
	using gclient_s = void;
	using gentity_s = void;
	using netadr_t = void;
	using PartyJoinInfo = void;
	using scrContext_t = void;
	using XHash = std::uint64_t;
}

namespace steam {
	using AccountID_t = std::uint32_t;
	using AppId_t = std::uint32_t;
	using ClientUnifiedMessageHandle = std::uint64_t;
	using DepotId_t = std::uint32_t;
	using HServerListRequest = void*;
	using HSteamPipe = int;
	using HSteamUser = int;
	using PartyBeaconID_t = std::uint64_t;
	using PublishedFileId_t = std::uint64_t;
	using ScreenshotHandle = std::uint32_t;
	using SteamAPICall_t = std::uint64_t;

	using SteamAPIWarningMessageHook_t = void(__cdecl*)(int, const char*);
	using SteamAPI_CheckCallbackRegistered_t = std::uint32_t(__cdecl*)(int callback_num);

	class ISteamAppList;
	class ISteamApps;
	class ISteamClient;
	class ISteamController;
	class ISteamFriends;
	class ISteamGameSearch;
	class ISteamGameServer;
	class ISteamGameServerStats;
	class ISteamHTMLSurface;
	class ISteamHTTP;
	class ISteamInput;
	class ISteamInventory;
	class ISteamMatchmaking;
	class ISteamMatchmakingPlayersResponse;
	class ISteamMatchmakingServerListResponse;
	class ISteamMatchmakingServers;
	class ISteamMusic;
	class ISteamMusicRemote;
	class ISteamNetworking;
	class ISteamParentalSettings;
	class ISteamParties;
	class ISteamRemotePlay;
	class ISteamRemoteStorage;
	class ISteamScreenshots;
	class ISteamUGC;
	class ISteamUnifiedMessages;
	class ISteamUser;
	class ISteamUserStats;
	class ISteamUtils;
	class ISteamVideo;
}

namespace t6s {
	using Glyph = void;
	using KerningPairs = void;
	using Material = void;
	using SndAliasList = void;
}

namespace t7s {
	using Glyph = void;
	using KerningPairs = void;
	using Material = void;

	class lua_Debug;
	class lua_State;

	using lua_Alloc = void*(*)(void*, void*, std::size_t, std::size_t);
	using lua_CFunction = int(*)(void*);
	using lua_Hook = void(*)(lua_State*, lua_Debug*);

	namespace hks {
		using DebugMapT = int(*)(const char* file_name, int lua_line);
		using EmergencyGCFailFuncT = void(*)(lua_State*, std::size_t);
		using LogFuncT = void(*)(lua_State*, const char*, ...);

		class Closure;
		class CClosure;
		class HashTable;
		class InternString;
		class Metatable;
		class Method;
		class StructInst;
		class UpValue;
		class UserData;
	}
}

namespace t8 {
	using ErrorCode = int;
}

namespace t9 {
	using Material = void;

	class _jmp_buf;
	class Font;
	class lua_Debug;
	class lua_State;
	class TTFDef;

	enum class JoinResult;

	using bdEntityID = std::uint64_t;
	using bdOnlineUserID = bdEntityID;
	using bdUByte8 = std::uint8_t;
	using bdUInt64 = std::uint64_t;
	using FontHandle = Font*;
	using hks_debug_map = int(*)(const char*, int);
	using hksBool = int;
	using hksByte = std::uint8_t;
	using hksBytecodeInstruction = std::uint32_t;
	using hksChar = char;
	using HksEmergencyGCFailFunc = void(*)(lua_State*, std::size_t);
	using hksInt32 = int;
	using hksInt64 = std::int64_t;
	using HksLogFunc = void(*)(lua_State*, const char*, ...);
	using HksNativeValueAsInt = std::uint32_t;
	using HksNumber = float;
	using hksShort16 = short;
	using hksSize = std::size_t;
	using hksUint32 = std::uint32_t;
	using hksUint64 = std::uint64_t;
	using hksUshort16 = std::uint16_t;
	using jmp_buf = _jmp_buf[1];
	using joinCompleteCallback = void(*)(const int, JoinResult);
	using LobbyUIScreen = int;
	using lua_Alloc = void*(*)(void*, void*, std::size_t, std::size_t);
	using lua_CFunction = hksInt32(*)(lua_State*);
	using lua_Hook = void(*)(lua_State*, lua_Debug*);
	using MaterialHandle = Material*;
	using XUID = bdOnlineUserID;

	namespace hks {
		class CallSite;
		class cclosure;
		class Debugger;
		class HashTable;
		class HksClosure;
		class InternString;
		class StructInst;
		class UserData;

		using ErrorHandler = CallSite*;
		using HksGcCost = int;
	}
}

namespace t10 {
	using gclient_s = void;
	using gentity_s = void;
	using netadr_t = void;
	using PartyJoinInfo = void;
	using scrContext_t = void;
	using XHash = std::uint64_t;
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
