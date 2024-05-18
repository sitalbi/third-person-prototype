// Fill out your copyright notice in the Description page of Project Settings.


#include "CanAttackAnimNotify.h"

void UCanAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AThirdPersonCharacter* Character = Cast<AThirdPersonCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->SetCanAttack();
		}
	}
}