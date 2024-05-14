// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackAnimNotifyState.h"

void UAttackAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	// Call the base class version of this function
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
}

void UAttackAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	// Call the base class version of this function
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (MeshComp)
	{
		AThirdPersonCharacter* Character = Cast<AThirdPersonCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->AttackHitDetection();
			
		}
	}
}

void UAttackAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// Call the base class version of this function
	Super::NotifyEnd(MeshComp, Animation);
}