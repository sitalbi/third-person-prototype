// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "ThirdPersonCharacter/ThirdPersonCharacter.h"
#include "TargetLockComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDPERSON_API UTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetLockComponent();


	enum class EDirection 
	{
		Left,
		Right
	};

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TArray<AActor*> TraceForTarget();

	AActor* GetTargetActor(TArray<AActor*> actors);
	AActor* GetTargetActorSwitch(TArray<AActor*> actors, EDirection direction);

	void SwitchTargetLock(const FInputActionValue& Value);

	void UpdateTargetLock();

public:	

	void TargetLockOn(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	bool GetIsLockedOn();

	UPROPERTY(EditAnywhere, Category = "Lock variables")
	float lockOnDistance = 1000.0f;
	UPROPERTY(EditAnywhere, Category = "Lock variables")
	float interpolationSpeed = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Lock variables")
	float distanceFactor = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Lock variables")
	float clampAngle = 35.0f;

	UPROPERTY(EditAnywhere, Category = "Lock variables")
	TSubclassOf<AActor>lockOnClass;

	/** Target Lock Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetLockAction;

	/** Switch target Lock Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchTargetLockAction;

	UFUNCTION(BlueprintCallable)
	FVector GetTargetLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LockWidgetClass;



private:

	FTimerHandle TargetLockTimerHandle;
	
	AActor* targetActor;

	AThirdPersonCharacter* playerCharacter;

	void SetLockTimer(bool IsLocked);

	FRotator GetLockOnRotation();

	void ChangeTargetActor(AActor* newTarget);

	bool isLockedOn = false;

	UUserWidget* lockWidget;

};

