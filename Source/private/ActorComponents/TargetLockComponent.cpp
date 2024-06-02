#include "Camera/CameraComponent.h"
#include "Math/Vector.h"
#include "ActorComponents/TargetLockComponent.h"
#include <InputTriggers.h>
#include <EnhancedInputComponent.h>
#include <Kismet/KismetMathLibrary.h>



// Sets default values for this component's properties
UTargetLockComponent::UTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();

	playerCharacter = Cast<AThirdPersonCharacter>(GetOwner());

	UEnhancedInputComponent* playerInputComponent = Cast<UEnhancedInputComponent>(playerCharacter->InputComponent);
	playerInputComponent->BindAction(TargetLockAction, ETriggerEvent::Triggered, this, &UTargetLockComponent::TargetLockOn);
	playerInputComponent->BindAction(SwitchTargetLockAction, ETriggerEvent::Triggered, this, &UTargetLockComponent::SwitchTargetLock);
}


void UTargetLockComponent::TargetLockOn(const FInputActionValue& Value)
{
	if (!isLockedOn) {
		TArray<AActor*> actors = TraceForTarget();
		if (actors.Num() > 0)
		{
			if (targetActor == nullptr) {
				targetActor = GetTargetActor(actors);
			}
			isLockedOn = targetActor != nullptr;

			if (isLockedOn) {
				SetLockTimer(true);
				playerCharacter->SetDefaultSpeed(); // stop sprinting
			}
		}
	}
	else
	{
		SetLockTimer(false);
	}
}

bool UTargetLockComponent::GetIsLockedOn()
{
	return isLockedOn;
}


TArray<AActor*> UTargetLockComponent::TraceForTarget()
{
	// Get all actors in a zone around the player using multi-sphere trace
	TArray<FHitResult> hitResults;
	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(playerCharacter);
	TArray<AActor*> actorsToReturn;

	FVector pos = playerCharacter->GetActorLocation();

	FCollisionShape shape = FCollisionShape::MakeSphere(lockOnDistance);

	bool hit = GetWorld()->SweepMultiByObjectType(
		hitResults,
		pos,
		pos,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		shape
	);


	if (hit)
	{
		for (FHitResult hitResult : hitResults)
		{
			AActor* actor = hitResult.GetActor();
			if (actor && !actorsToIgnore.Contains(actor) && !actorsToReturn.Contains(actor))
			{
				// check if actor is child of lockOnClass
				if (actor->IsA(lockOnClass)) {
					actorsToReturn.Add(actor);
				}
			}
		}
	}

	// Debug draw the sphere
	//DrawDebugSphere(GetWorld(), pos, lockOnDistance, 12, actorsToReturn.Num() > 0 ? FColor::Green : FColor::Red, false, 5.f);

	return actorsToReturn;

}

AActor* UTargetLockComponent::GetTargetActor(TArray<AActor*> actors) {
	AActor* target = nullptr;

	double minDot = -1.0;

	for (AActor* actor : actors)
	{
		if (!actor) continue;
		// line trace from the camera towards the actor for a distance of lockOnDistance
		FHitResult hitResult;
		FVector start = playerCharacter->GetFollowCamera()->GetComponentLocation();
		FVector end = actor->GetActorLocation();

		FCollisionQueryParams params;
		params.AddIgnoredActor(playerCharacter);

		bool hit = GetWorld()->LineTraceSingleByObjectType(
			hitResult,
			start,
			end,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
			params
		);

		// Check the closest actor to the center of the screen
		FRotator r = UKismetMathLibrary::FindLookAtRotation(playerCharacter->GetActorLocation(), actor->GetActorLocation());

		double dot = FVector::DotProduct(playerCharacter->GetFollowCamera()->GetForwardVector(), r.Vector());

		if (dot > minDot)
		{
			minDot = dot;
			target = actor;
		}
	}
	// Debug draw the line
	//DrawDebugLine(GetWorld(), playerCharacter->GetFollowCamera()->GetComponentLocation(), target->GetActorLocation(), FColor::Red, false, 5.f);

	return target;
}

AActor* UTargetLockComponent::GetTargetActorSwitch(TArray<AActor*> actors, EDirection direction)
{
	AActor* target = nullptr;
	double minDot = -1.0;

	for (AActor* actor : actors)
	{
		if (!actor) continue;

		FHitResult hitResult;
		FVector start = playerCharacter->GetFollowCamera()->GetComponentLocation();
		FVector end = actor->GetActorLocation();

		FCollisionQueryParams params;
		params.AddIgnoredActor(playerCharacter);
		if (targetActor != nullptr) params.AddIgnoredActor(targetActor);

		bool hit = GetWorld()->LineTraceSingleByObjectType(
			hitResult,
			start,
			end,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
			params
		);

		double dot = FVector::DotProduct(playerCharacter->GetFollowCamera()->GetRightVector(), hitResult.Normal);
		bool isCorrectSide = (direction == EDirection::Left) ? (asin(dot) > 0) : (asin(dot) < 0);

		if (isCorrectSide)
		{
			FRotator r = UKismetMathLibrary::FindLookAtRotation(playerCharacter->GetActorLocation(), actor->GetActorLocation());
			dot = FVector::DotProduct(playerCharacter->GetFollowCamera()->GetForwardVector(), r.Vector());

			if (dot > minDot)
			{
				minDot = dot;
				target = actor;
			}
		}
	}

	return target;
}


void UTargetLockComponent::SwitchTargetLock(const FInputActionValue& Value)
{
	if (isLockedOn) {
		FVector2D InputAxisVector = Value.Get<FVector2D>();

		AActor* newTarget = nullptr;
		EDirection direction = InputAxisVector.X > 0 ? EDirection::Right : EDirection::Left;
		newTarget = GetTargetActorSwitch(TraceForTarget(), direction);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("New Target: %s"), newTarget ? *newTarget->GetName() : TEXT("None")));

		if (newTarget != nullptr) {
			targetActor = newTarget;
		}
	}
}

void UTargetLockComponent::UpdateTargetLock()
{
	if (!isLockedOn || targetActor == nullptr)
	{
		SetLockTimer(false);
		return;
	}
	else
	{
		if (!IsValid(targetActor)) {
			targetActor = nullptr;
			return;
		}
		FVector start = playerCharacter->GetActorLocation();
		FVector end = targetActor->GetActorLocation();

		float distance = FVector::Dist(start, end);

		if (distance > lockOnDistance) {
			targetActor = nullptr;
			isLockedOn = false;
			SetLockTimer(false);
		}
		else {
			FRotator currentRotation = playerCharacter->GetController()->GetControlRotation();

			FRotator targetRotation = GetLockOnRotation();

			float deltaTime = GetWorld()->GetDeltaSeconds();
			float interpSpeed = interpolationSpeed;

			if (playerCharacter->GetIsEquipped()) {
				// Interpolate the player's rotation smoothly
				FRotator currentPlayerRotation = playerCharacter->GetActorRotation();
				FRotator targetPlayerRotation = FRotator(0, targetRotation.Yaw, 0);

				FRotator newPlayerRotation = FMath::RInterpTo(currentPlayerRotation, targetPlayerRotation, deltaTime, interpSpeed * 2.0f);

				// Set the interpolated rotation to the player
				playerCharacter->SetActorRotation(newPlayerRotation);
			}

			FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotation, deltaTime, interpSpeed);

			// Set the rotation of the camera
			playerCharacter->GetController()->SetControlRotation(newRotation);



		}
	}

}


void UTargetLockComponent::SetLockTimer(bool IsLocked)
{
	if (IsLocked) {
		GetWorld()->GetTimerManager().SetTimer(TargetLockTimerHandle, this, &UTargetLockComponent::UpdateTargetLock, GetWorld()->GetDeltaSeconds(), true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TargetLockTimerHandle);
		targetActor = nullptr;
		isLockedOn = false;
	}
}

FRotator UTargetLockComponent::GetLockOnRotation()
{
	FVector start = playerCharacter->GetFollowCamera()->GetComponentLocation();

	FVector targetPos = targetActor->GetActorLocation();

	double dist = FVector::Dist(start, targetPos);

	FVector end = FVector(targetPos.X, targetPos.Y, targetPos.Z - (dist / distanceFactor));

	FVector Direction = end - start;

	FRotator Rot = FRotationMatrix::MakeFromX(Direction).Rotator();


	return Rot;
}
