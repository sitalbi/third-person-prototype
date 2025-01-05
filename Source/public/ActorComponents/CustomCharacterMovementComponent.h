#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "ThirdPersonCharacter/ThirdPersonCharacter.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSON_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DashMontage;


	bool IsSprinting() const { 
		return bIsSprinting && Velocity.Size() > DefaultSpeed;
	}
	bool IsDashing() const { 
		return bIsDashing; 
	}

	void StartSprint();
	void StopSprint();

	void SetOrientationToMovement(bool orientation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float DefaultSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float SprintSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float MaxStamina = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float Stamina;									   
															   
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float SprintStaminaDrainRate = 1.0f;					   
															   
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float StaminaRecoveryRate = 1.0f;						   
															   
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float DodgeStaminaDrain = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float DashCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats)
	float DashForce = 1000.0f;

	bool bIsSprinting;

	AThirdPersonCharacter* player;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateSprint();
	void RecoverStamina();

	void Dash();
	void ResetDash();

	bool bIsDashing;


	FTimerHandle SprintTimerHandle;
	FTimerHandle DashTimerHandle;
	FTimerHandle RecoverStaminaTimerHandle;
};
