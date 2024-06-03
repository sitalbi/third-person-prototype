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

	UPROPERTY(EditAnywhere, Category = "Controller")
	FName Name;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float Health;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDamage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	int HitCount = 0;

	UWidgetComponent* HealthBar;

public:
	// override MeleeHitInterface
	virtual void OnMeleeHit(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
