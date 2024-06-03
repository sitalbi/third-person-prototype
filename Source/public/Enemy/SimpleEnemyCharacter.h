// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "SimpleEnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSON_API ASimpleEnemyCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
	

public:
	ASimpleEnemyCharacter();

	virtual void OnMeleeHit(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


};
