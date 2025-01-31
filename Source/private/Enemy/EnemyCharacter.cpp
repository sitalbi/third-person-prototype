#include "Enemy/EnemyCharacter.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Components/CapsuleComponent.h"

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
		HealthBar->SetVisibility(false);
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

void AEnemyCharacter::DestroyActor()
{
	Destroy();
}

void AEnemyCharacter::ActivateRagdoll()
{
	// Activate ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
	GetMesh()->bBlendPhysics = true;

	// Destroy the actor
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemyCharacter::DestroyActor, 5.0f, false);
}

void AEnemyCharacter::Death()
{
	// Stop the tick function
	SetActorTickEnabled(false);
	HealthBar->SetVisibility(false);
	m_isDead = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(false);

	if (GetController())
	{
		GetController()->UnPossess();
	} 

	if (DeathMontage) {
		// rotate the enemy to face the player
		FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());
		SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance)
		{
			ActivateRagdoll();

			// add force in the direction of the attack
			FVector Force = -GetActorForwardVector() * 3500000.0f;
			GetMesh()->AddForce(Force);
		}

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
	if(!isLockedOn && (animInstance && !animInstance->Montage_IsPlaying(HitMontage)))
	{
		HealthBar->SetVisibility(false);
	}
	 
}

bool AEnemyCharacter::IsKnockedDown()
{
	return isKnockedDown;
}


void AEnemyCharacter::OnMeleeHit(FHitResult HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Melee Hit"));

}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && !animInstance->Montage_IsPlaying(HitMontage) && !animInstance->Montage_IsPlaying(KnockdownMontage))
	{
		Health -= DamageAmount;
		Health = fmax(Health, 0);
		if (Health == 0)
		{
			if (!m_isDead) {
				Death();
			}
		}
		// Event On Damage Taken broadcast
		OnDamageTaken.Broadcast();
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

bool AEnemyCharacter::isDead()
{
	return m_isDead;
}
