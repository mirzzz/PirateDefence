// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxPirateDefenceGameMode.h"
#include "NxPirateDefenceCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANxPirateDefenceGameMode::ANxPirateDefenceGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
