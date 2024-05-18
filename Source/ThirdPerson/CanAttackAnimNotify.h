// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ThirdPersonCharacter.h"
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CanAttackAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSON_API UCanAttackAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

	public:
		virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
