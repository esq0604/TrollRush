// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrollRushGameMode.h"
#include "TrollRushCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATrollRushGameMode::ATrollRushGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
