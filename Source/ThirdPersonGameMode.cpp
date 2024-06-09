// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonGameMode.h"
#include "ThirdPersonCharacter/ThirdPersonCharacter.h"
#include "UObject/ConstructorHelpers.h"

AThirdPersonGameMode::AThirdPersonGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerPawnBPClass: %s"), *PlayerPawnBPClass.Class->GetName());
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
