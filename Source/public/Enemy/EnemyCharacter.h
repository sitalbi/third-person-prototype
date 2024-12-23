// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Components/WidgetComponent.h>
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy/MeleeHitInterface.h"
#include "EnemyCharacter.generated.h"




UCLASS()
class THIRDPERSON_API AEnemyCharacter : public ACharacter, public IMeleeHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackBaseDamage;


	bool isLockedOn = false;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackEnded OnAttackMontageEnded;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void DestroyActor();

	void ActivateRagdoll();

	void Death();
	
	int HitCount = 0;

	UWidgetComponent* HealthBar;

	bool m_isDead = false;

public:
	// override MeleeHitInterface
	virtual void OnMeleeHit(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void SetIsLockedOn(bool bShow);

	UFUNCTION(BlueprintCallable)
	virtual void Attack();

	void AttackEnd(UAnimMontage* Montage, bool bInterrupted);

	bool isDead();



};
