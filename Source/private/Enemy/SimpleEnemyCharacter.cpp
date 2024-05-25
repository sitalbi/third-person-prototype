#include "Enemy/SimpleEnemyCharacter.h"

ASimpleEnemyCharacter::ASimpleEnemyCharacter()
{
}

void ASimpleEnemyCharacter::OnMeleeHit(FHitResult HitResult)
{

	// TODO: make the character face the direction of the hit

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage))
	{
		animInstance->Montage_Play(HitMontage, 1.0);

		FName section = FName(*FString::Printf(TEXT("Hit%d"), HitCount + 1));
		animInstance->Montage_JumpToSection(section);
		HitCount = (HitCount + 1) % animInstance->GetCurrentActiveMontage()->GetNumSections();

		
	}
}
