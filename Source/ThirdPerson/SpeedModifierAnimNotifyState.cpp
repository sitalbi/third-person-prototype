// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedModifierAnimNotifyState.h"

void USpeedModifierAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (MeshComp)
	{
		AThirdPersonCharacter* Character = Cast<AThirdPersonCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->SetSpeed(modifiedSpeed);
		}
	}
}

void USpeedModifierAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AThirdPersonCharacter* Character = Cast<AThirdPersonCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->SetDefaultSpeed();
		}
	}
}