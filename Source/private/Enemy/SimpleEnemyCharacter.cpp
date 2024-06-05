#include "Enemy/SimpleEnemyCharacter.h"
#include <Kismet/KismetMathLibrary.h>

ASimpleEnemyCharacter::ASimpleEnemyCharacter()
{
}

void ASimpleEnemyCharacter::OnMeleeHit(FHitResult HitResult)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage))
	{
		animInstance->Montage_Play(HitMontage, 1.0);

		FName section = FName(*FString::Printf(TEXT("Hit%d"), HitCount + 1));
		animInstance->Montage_JumpToSection("Hit3");
		HitCount = (HitCount + 1) % animInstance->GetCurrentActiveMontage()->GetNumSections();
	}
}

float ASimpleEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageDealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Rotate the enemy to face the player (damage causer)
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation());
	SetActorRotation(FRotator(0, LookAtRotation.Yaw, 0));

	return DamageDealt;
}
