
#include "ActorComponents/TargetLockComponent.h"
#include <InputTriggers.h>
#include <EnhancedInputComponent.h>


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
}

// Called every frame
void UTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

void UTargetLockComponent::TargetLockOn()
{
}


TArray<AActor*> UTargetLockComponent::TraceForTarget()
{
	// TODO: Get all actors in a zone around the player
	return TArray<AActor*>();
}
