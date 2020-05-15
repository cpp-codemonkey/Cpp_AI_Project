// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Cpp_AIGameMode.h"
#include "Cpp_AICharacter.h"
#include "UObject/ConstructorHelpers.h"

ACpp_AIGameMode::ACpp_AIGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
