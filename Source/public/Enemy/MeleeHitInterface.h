// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeHitInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMeleeHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THIRDPERSON_API IMeleeHitInterface
{
	GENERATED_BODY()

public:

	virtual void OnMeleeHit(FHitResult HitResult) = 0;
};
