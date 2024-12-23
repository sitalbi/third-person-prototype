// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "AttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDPERSON_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AttackHitDetection();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	float WeaponDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* WeaponCollision;

private:

	UStaticMeshComponent* WeaponMeshComponent;

};
