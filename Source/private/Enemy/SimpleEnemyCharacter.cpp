#include "Enemy/SimpleEnemyCharacter.h"
#include <Kismet/KismetMathLibrary.h>

ASimpleEnemyCharacter::ASimpleEnemyCharacter()
{
}

void ASimpleEnemyCharacter::OnMeleeHit(FHitResult HitResult)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage) && !animInstance->Montage_IsPlaying(DeathMontage) && !animInstance->Montage_IsPlaying(KnockdownMontage))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyHit"));
		animInstance->Montage_Play(HitMontage, 1.0);

		FName section = FName(*FString::Printf(TEXT("Hit%d"), HitCount + 1));
		animInstance->Montage_JumpToSection(section);
		HitCount = (HitCount + 1) % 3;
	}
}

float ASimpleEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageDealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// If the enemy is already dead, return
	if (Health == 0)
	{
		return DamageDealt;
	}

	// Rotate the enemy to face the player (damage causer)
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation());
	SetActorRotation(FRotator(0, LookAtRotation.Yaw, 0));

	if (DamageDealt >= KnockdownDamageThreshold)
	{

		// Play the knockdown montage
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance && !animInstance->Montage_IsPlaying(KnockdownMontage))
		{
			animInstance->Montage_Play(KnockdownMontage, 1.0);
			KnockdownCount = rand() % 2;
			FName section = FName(*FString::Printf(TEXT("Knockdown%d"), KnockdownCount + 1));
			animInstance->Montage_JumpToSection(section);
			// Set timer 
			GetWorld()->GetTimerManager().SetTimer(KnockdownTimerHandle, this, &ASimpleEnemyCharacter::EndKnockdown, DamageDealt/10, false);
			isKnockedDown = true;
		}
	}

	return DamageDealt;
}

void ASimpleEnemyCharacter::EndKnockdown()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance) {
		FName section = FName(*FString::Printf(TEXT("Knockdown_GetUp%d"), KnockdownCount + 1));
		animInstance->Montage_Play(KnockdownMontage, 1.0);
		animInstance->Montage_JumpToSection(section);
		// Set delegate to end of anim montage
		OnKnockdownEndDelegate.BindUObject(this, &ASimpleEnemyCharacter::ResetKnockDown);
		animInstance->Montage_SetEndDelegate(OnKnockdownEndDelegate, KnockdownMontage);
	}
}

void ASimpleEnemyCharacter::ResetKnockDown(UAnimMontage* Montage, bool bInterrupted)
{
	isKnockedDown = false;
}
