#pragma once
#include "engine/engine_common.hpp"

namespace s2 {
	enum class ItemLockStatus {
		ItemLockStatus_Unlocked = 0,
		ItemLockStatus_Lock_ChallengeNotCompleted = 1,
		ItemLockStatus_Lock_LevelNotReached = 2,
		ItemLockStatus_Lock_OnlineDataNotFetched = 3,
		ItemLockStatus_Lock_AllLocked = 4,
		ItemLockStatus_Lock_InvalidIntenvoryStatus = 5,
		ItemLockStatus_Lock_EntitlementNotUnlocked = 6,
		ItemLockStatus_Lock_ExtLevelNotReached = 7,
		ItemLockStatus_Lock_ExtPrestigeLevelNotReached = 8,
		ItemLockStatus_Lock_ExtinctionEscapesNotReached = 9,
		ItemLockStatus_Lock_ExtRelicEscapesNotReached = 10,
		ItemLockStatus_Lock_ExtKillsNotReached = 11,
		ItemLockStatus_Lock_ExtRevivesNotReached = 12,
		ItemLockStatus_Lock_PrestigeNotReached = 13,
		ItemLockStatus_Hidden_NotInInventory = 14,
		ItemLockStatus_Hidden_Unknown = 15
	};
}
