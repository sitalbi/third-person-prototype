// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonCharacter/EquipAnimNotify.h"

void UEquipAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AThirdPersonCharacter* Character = Cast<AThirdPersonCharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->Equip();
		}
	}
}