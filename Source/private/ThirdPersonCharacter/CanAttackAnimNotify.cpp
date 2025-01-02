#include "ThirdPersonCharacter/CanAttackAnimNotify.h"

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