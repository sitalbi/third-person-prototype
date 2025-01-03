#include <ActorComponents/CustomCharacterMovementComponent.h>
#include <ActorComponents/TargetLockComponent.h>
#include <EnhancedInputComponent.h>

void UCustomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	this->MaxWalkSpeed = DefaultSpeed;
	Stamina = MaxStamina;

	player = Cast<AThirdPersonCharacter>(GetOwner());

	UEnhancedInputComponent* playerInputComponent = Cast<UEnhancedInputComponent>(player->InputComponent);
	playerInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &UCustomCharacterMovementComponent::StartSprint);
	playerInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &UCustomCharacterMovementComponent::StopSprint);
	playerInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &UCustomCharacterMovementComponent::Dash);
}

void UCustomCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCustomCharacterMovementComponent::StartSprint()
{
	this->MaxWalkSpeed = SprintSpeed;
	bIsSprinting = true;
	GetWorld()->GetTimerManager().SetTimer(SprintTimerHandle, this, &UCustomCharacterMovementComponent::UpdateSprint, GetWorld()->GetDeltaSeconds(), true, 1.0f);
	if (player->IsTargetLocked())
	{
		SetOrientationToMovement(true);
	}
}

void UCustomCharacterMovementComponent::StopSprint()
{
	this->MaxWalkSpeed = DefaultSpeed;
	bIsSprinting = false;
	GetWorld()->GetTimerManager().SetTimer(SprintTimerHandle, this, &UCustomCharacterMovementComponent::RecoverStamina, GetWorld()->GetDeltaSeconds(), true, 1.0f);
	if (player->IsTargetLocked())
	{
		SetOrientationToMovement(false);
	}
}



void UCustomCharacterMovementComponent::UpdateSprint()
{
	// Get current character speed (not max speed)
	float currentSpeed = Velocity.Size();


	if (Stamina > 0)
	{
		if (bIsSprinting)
		{
			Stamina -= GetWorld()->GetDeltaSeconds() * SprintStaminaDrainRate;
		}
	}
	else
	{
		StopSprint();
	}
}

void UCustomCharacterMovementComponent::RecoverStamina()
{
	if (Stamina < MaxStamina)
	{
		Stamina += GetWorld()->GetDeltaSeconds() * StaminaRecoveryRate;
	}
	else
	{
		Stamina = MaxStamina;
		GetWorld()->GetTimerManager().ClearTimer(SprintTimerHandle);
	}
}

void UCustomCharacterMovementComponent::Dash()
{
	if (!bIsDashing && Stamina - DodgeStaminaDrain > 0) {
		UAnimInstance* animInstance = player->GetMesh()->GetAnimInstance();
		if (DashMontage) {
			if (!IsFalling()) {
				animInstance->Montage_Play(DashMontage, 1.0f);
			}
		}

		// Get the input movement direction
		FVector dashDirection = player->GetLastMovementInputVector();
		
		if (dashDirection.Size() == 0.0) {
			dashDirection = -player->GetActorForwardVector();
		}
		
		// Normalize the direction and multiply by the dash force
		FVector dashVelocity = dashDirection.GetSafeNormal() * DashForce;
		Launch(dashVelocity);
		Stamina -= DodgeStaminaDrain;
		bIsDashing = true;
		GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &UCustomCharacterMovementComponent::ResetDash, DashCooldown, false);
	}
}

void UCustomCharacterMovementComponent::ResetDash()
{
	bIsDashing = false;
}

void UCustomCharacterMovementComponent::SetOrientationToMovement(bool orientation)
{
	if (orientation) {
		this->bOrientRotationToMovement = true;
		this->bUseControllerDesiredRotation = false;
	}
	else {
		this->bOrientRotationToMovement = false;
		this->bUseControllerDesiredRotation = true;
	}
	
}
