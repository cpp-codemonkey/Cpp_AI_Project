#pragma once

#include "Runtime/Core/Public/UObject/NameTypes.h"
#include "Runtime/Core/Public/Containers/UnrealString.h"

namespace bb_keys
{
	TCHAR const * const target_location = TEXT("TargetLocation");
	TCHAR const* const can_see_player = TEXT("CanSeePlayer");
	TCHAR const* const patrol_path_vector = TEXT("PatrolPathVector");
	TCHAR const* const patrol_path_index = TEXT("PatrolPathIndex");
	TCHAR const* const player_is_in_melee_range = TEXT("PlayerIsInMeleeRange");
	TCHAR const* const is_investigating = TEXT("IsInvestigating");
}