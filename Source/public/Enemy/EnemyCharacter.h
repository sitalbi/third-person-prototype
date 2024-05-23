// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:

	UPROPERTY(EditAnywhere, Category = "Controller")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Health;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Damage;

public:
	// override MeleeHitInterface
	virtual void OnMeleeHit(FHitResult HitResult) override;
};
