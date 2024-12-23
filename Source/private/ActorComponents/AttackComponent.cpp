// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include <Enemy/MeleeHitInterface.h>
#include <Kismet/GameplayStatics.h>

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAttackComponent::AttackHitDetection()
{
	FVector StartLocation = WeaponMeshComponent->GetSocketLocation("Start");
	FVector EndLocation = WeaponMeshComponent->GetSocketLocation("End");

	float Radius = 20.0f;
	float HalfHeight = (EndLocation - StartLocation).Size() / 2;
	FQuat Rotation = WeaponMeshComponent->GetSocketQuaternion("End");

	FCollisionQueryParams QueryParams;
	//QueryParams.AddIgnoredActor(this); // Ignore self
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByObjectType(
		HitResult,
		StartLocation,
		EndLocation,
		Rotation,
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(Radius, HalfHeight),
		QueryParams
	);

	if (bHit) {
		AActor* HitActor = HitResult.GetActor();

		//float DamageAmount = WeaponDamage * AttackMultiplier;

		/*IMeleeHitInterface* MeleeHitActor = Cast<IMeleeHitInterface>(HitActor);
		if (MeleeHitActor) {
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				DamageAmount,
				GetActorLocation(),
				HitResult,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);
			MeleeHitActor->OnMeleeHit(HitResult);

		}*/
	}

	// Debug
	// DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, HalfHeight, Radius, Rotation, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
}

