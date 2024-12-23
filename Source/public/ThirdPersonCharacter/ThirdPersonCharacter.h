// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include <Components/BoxComponent.h>
#include <GameFramework/Character.h>
#include "Logging/LogMacros.h"
#include "ThirdPersonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UTargetLockComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Draw Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DrawAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	/** Heavy Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	/** Roll Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RollAction;

	


	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DrawMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* JumpAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* RollMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	float WeaponDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attack, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* WeaponCollision;


public:
	AThirdPersonCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug, meta = (AllowPrivateAccess = "true"))
	bool Debug = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float defaultSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float sprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float heavyAttackMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float maxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float maxStamina = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float stamina = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float sprintStaminaDrainRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float staminaRecoveryRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float rollStaminaDrain = 1.0f;

	UFUNCTION(BlueprintCallable)
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	void SetSpeed(float speed);
	void SetDefaultSpeed();
	void SetCanAttack();

	void SetOrientRotationToMovement(bool orientation);

	UFUNCTION(BlueprintCallable)
	bool GetIsEquipped();

	UFUNCTION(BlueprintCallable)
	bool GetIsRolling();


	void Equip();
	void AttackHitDetection();
	
	virtual void Landed(const FHitResult& Hit) override;


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for sprint input */
	void Sprint(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for drawing input */
	void Draw(const FInputActionValue& Value);

	/** Called for attacking input */
	void Attack(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);

	/** Called for rolling input */
	void Roll(const FInputActionValue& Value);

	/** Check for animation playing */
	bool IsPlayingMontage();

	void ResetAttack();

	void UpdateSprint();

	void RecoverStamina();

	void ResetTimeDilation();
	
	UStaticMeshComponent* WeaponMeshComponent;

	UTargetLockComponent* TargetLockComponent;

	unsigned int AttackCount = 0;

	bool canAttack = true;

	float AttackMultiplier = 1.0f;

	bool IsEquipped = false;
	bool isRolling = false;

	FVector RollDirection;

	FOnMontageEnded OnAttackEndDelegate;

	FTimerHandle SprintTimerHandle;
	FTimerHandle SlowMotionTimerHandle;

	bool hit = false;


	


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

