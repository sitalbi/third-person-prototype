// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyCharacter.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	HitCount = 0;
	Health = MaxHealth;

	HealthBar = FindComponentByClass<UWidgetComponent>();

	if (HealthBar) {
		UE_LOG(LogTemp, Warning, TEXT("HealthBar Found"));
		HealthBar->SetVisibility(false);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("HealthBar Not Found"));
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		// Bind the delegate directly to montage end event
		FOnMontageEnded MontageDelegate;
		MontageDelegate.BindUObject(this, &AEnemyCharacter::AttackEnd);
		AnimInstance->Montage_SetEndDelegate(MontageDelegate, AttackMontage);
	}

}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// rotate health bar to face player
	if (HealthBar && HealthBar->IsVisible()) {
		// get camera location
		FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();

		// look at camera
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(HealthBar->GetComponentLocation(), CameraLocation);
		HealthBar->SetWorldRotation(LookAtRotation);
	}

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && animInstance->Montage_IsPlaying(HitMontage) && !isLockedOn)
	{
		HealthBar->SetVisibility(true);
	}
	if(!isLockedOn && !animInstance->Montage_IsPlaying(HitMontage))
	{
		HealthBar->SetVisibility(false);
	}
	 
}


void AEnemyCharacter::OnMeleeHit(FHitResult HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Melee Hit"));

}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage))
	{
		Health -= DamageAmount;
		Health = fmax(Health, 0);
		if (Health == 0)
		{
			Destroy();
		}
	}

	return DamageAmount;
}

void AEnemyCharacter::SetIsLockedOn(bool bShow)
{
	isLockedOn = bShow;
	HealthBar->SetVisibility(bShow);
}

void AEnemyCharacter::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack"));
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage))
	{
		animInstance->Montage_Play(AttackMontage, 1.0);
		// Bind the delegate directly to montage end event
		FOnMontageEnded MontageDelegate;
		MontageDelegate.BindUObject(this, &AEnemyCharacter::AttackEnd);
		animInstance->Montage_SetEndDelegate(MontageDelegate, AttackMontage);
	}
}

void AEnemyCharacter::AttackEnd(UAnimMontage* Montage, bool bInterrupted)
{
	OnAttackMontageEnded.Broadcast();
}
