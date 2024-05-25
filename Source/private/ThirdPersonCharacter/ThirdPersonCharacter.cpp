 // Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonCharacter/ThirdPersonCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Enemy/MeleeHitInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AThirdPersonCharacter

AThirdPersonCharacter::AThirdPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
}

void AThirdPersonCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (StaticMeshComponent->GetName() == "WeaponDraw")
		{
			DrawComponent = StaticMeshComponent;
		}
		else if (StaticMeshComponent->GetName() == "WeaponSheath")
		{
			SheathComponent = StaticMeshComponent;
		}
	}
	Equip();
	DrawComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Move);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AThirdPersonCharacter::Sprint);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Look);

		// Drawing
		EnhancedInputComponent->BindAction(DrawAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Draw);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Attack);

		// Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::HeavyAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &AThirdPersonCharacter::HeavyAttack);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AThirdPersonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		// if the character is attacking or drawing weapon, he can't move
		if (!canAttack || animInstance->Montage_IsPlaying(DrawMontage)) {
			return;
		}

		// find out which way is forward
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);


		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AThirdPersonCharacter::Sprint(const FInputActionValue& Value)
{
	// input is a boolean
	bool bSprint = Value.Get<bool>();

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (bSprint && !GetCharacterMovement()->IsFalling() && !IsPlayingMontage())
	{
		SetSpeed(sprintSpeed);
	}
	else
	{
		SetDefaultSpeed();
	}

}


void AThirdPersonCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AThirdPersonCharacter::Draw(const FInputActionValue& Value)
{
	// input is a boolean
	bool bDraw = Value.Get<bool>();

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance) {
		if (IsEquipped)
		{
			animInstance->Montage_Play(DrawMontage, 1.0f);
			animInstance->Montage_JumpToSection("Sheath");
			IsEquipped = false;
		}
		else
		{
			animInstance->Montage_Play(DrawMontage, 1.0f);
			animInstance->Montage_JumpToSection("Draw");
			IsEquipped = true;
		}
	}
	
}

void AThirdPersonCharacter::Attack(const FInputActionValue& Value)
{
	// input is a boolean
	bool bAttack = Value.Get<bool>();

	if (bAttack)
	{
		if (!IsEquipped) {
			IsEquipped = true;
			Equip();
		}

		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (canAttack && animInstance) {  // Ensure animInstance is valid
			// make the player face the direction of the input
			if (Controller != nullptr)
			{
				// Get the movement input vector
				FVector MoveDirection = GetLastMovementInputVector();
				if (MoveDirection.Size() < 0.1) {
					MoveDirection = GetActorForwardVector();
				}
				FRotator NewRotation = MoveDirection.Rotation();
				NewRotation.Pitch = 0; // Ensure we only rotate around the Yaw axis
				SetActorRotation(NewRotation);
			}

			if (!GetMovementComponent()->IsFalling()) {
				canAttack = false;
				animInstance->Montage_Play(AttackMontage, 1.0f);

				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AThirdPersonCharacter::OnAttackMontageEnded);
				animInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

				FName section = FName(*FString::Printf(TEXT("Attack%d"), AttackCount + 1));
				animInstance->Montage_JumpToSection(section);
				AttackCount = (AttackCount + 1) % animInstance->GetCurrentActiveMontage()->GetNumSections();
			}
			else {
				canAttack = false;
				animInstance->Montage_Play(JumpAttackMontage, 1.0f);
			}
		}
	}
}


void AThirdPersonCharacter::HeavyAttack(const FInputActionValue& Value) {
	bool bHeavyAttack = Value.Get<bool>();

	UE_LOG(LogTemp, Warning, TEXT("Heavy Attack: %d"), bHeavyAttack);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (bHeavyAttack)
	{
		if (!IsEquipped) {
			IsEquipped = true;
			Equip();
		}
		if (canAttack) {
			canAttack = false;
			AnimInstance->Montage_Play(HeavyAttackMontage, 1.0f);
		}
	}
	else {
		if (AnimInstance->Montage_IsPlaying(HeavyAttackMontage)) {
			FString CurrentSectionName;
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(HeavyAttackMontage);
			if (MontageInstance)
			{
				CurrentSectionName = MontageInstance->GetCurrentSection().ToString();
			}
			UE_LOG(LogTemp, Warning, TEXT("Current Section: %s"), *CurrentSectionName);
			if (CurrentSectionName.Equals("Mid")) {
				
				AnimInstance->Montage_Play(HeavyAttackMontage, 1.0f);
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AThirdPersonCharacter::OnAttackMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, HeavyAttackMontage);
				AnimInstance->Montage_JumpToSection("End");
			}
			else {
				AnimInstance->Montage_Stop(0.5f, HeavyAttackMontage);
				ResetAttack();
			}
		}
	}
}

bool AThirdPersonCharacter::IsPlayingMontage()
{
	UAnimInstance * animInstance = GetMesh()->GetAnimInstance();
	return animInstance->Montage_IsPlaying(AttackMontage) || animInstance->Montage_IsPlaying(DrawMontage);
}

void AThirdPersonCharacter::Equip()
{
	if (IsEquipped)
	{
		DrawComponent->SetVisibility(true);
		SheathComponent->SetVisibility(false);
	}
	else
	{
		DrawComponent->SetVisibility(false);
		SheathComponent->SetVisibility(true);
	}
}

void AThirdPersonCharacter::AttackHitDetection()
{
	FVector StartLocation = DrawComponent->GetSocketLocation("Start");
	FVector EndLocation = DrawComponent->GetSocketLocation("End");

	float Radius = 20.0f;
	float HalfHeight = (EndLocation - StartLocation).Size() / 2;
	FQuat Rotation = DrawComponent->GetSocketQuaternion("End");

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore self
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
		
		IMeleeHitInterface* MeleeHitActor = Cast<IMeleeHitInterface>(HitActor);
		if (MeleeHitActor) {
			MeleeHitActor->OnMeleeHit(HitResult);
		}
	}

	// Debug
	DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, HalfHeight, Radius, Rotation, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
}


// use this instead of Landed event which is not triggered when landing due to a bug
void AThirdPersonCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (PrevMovementMode == EMovementMode::MOVE_Falling && GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
	{
		if (!canAttack) {
			UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
			animInstance->Montage_Play(JumpAttackMontage, 1.0f);
			animInstance->Montage_JumpToSection("Land");
			ResetAttack();
		}
	}

	
}


void AThirdPersonCharacter::ResetAttack()
{
	canAttack = true;
	AttackCount = 0;
}

void AThirdPersonCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Attack Montage Interrupted"));
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Attack Montage Ended"));
		ResetAttack();
	}
}

void AThirdPersonCharacter::SetSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

void AThirdPersonCharacter::SetDefaultSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = defaultSpeed;
}

void AThirdPersonCharacter::SetCanAttack()
{
	this->canAttack = true;
}

