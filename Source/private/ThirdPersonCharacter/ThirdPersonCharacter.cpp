 // Copyright Epic Games, Inc. All Rights Reserved.


#include "ThirdPersonCharacter/ThirdPersonCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Enemy/MeleeHitInterface.h"
#include <Kismet/GameplayStatics.h>
#include <ActorComponents/TargetLockComponent.h>
#include <ActorComponents/CustomCharacterMovementComponent.h>
#include <Enemy/EnemyCharacter.h>
#include <Kismet/KismetMathLibrary.h>

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

	TimeDilationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimeDilationTimeline"));

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
		if (StaticMeshComponent->GetName() == "Weapon")
		{
			WeaponMeshComponent = StaticMeshComponent;
		}
	}
	Equip();
	WeaponMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	
	TargetLockComponent = Cast<UTargetLockComponent>(GetComponentByClass(UTargetLockComponent::StaticClass()));
	
	CustomCharacterMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

	health = maxHealth;

	if (TimeDilationCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleTimeDilation"));

		TimeDilationTimeline->AddInterpFloat(TimeDilationCurve, ProgressFunction);
		// Timeline Ignore Time Dilation
		TimeDilationTimeline->SetIgnoreTimeDilation(true);
		TimeDilationTimeline->SetTimelineLength(TimeDilationCurve->FloatCurve.GetLastKey().Time);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Move);

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

void AThirdPersonCharacter::Jump()
{
	if (CanJump())
	{
		Super::Jump();
	}
}

bool AThirdPersonCharacter::CanJump() const
{
	return Super::CanJump() && canAttack && !GetMovementComponent()->IsFalling() && !IsPlayingMontage();
}

bool AThirdPersonCharacter::CanMove() const
{
	return canMove;
}

void AThirdPersonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && CanMove())
	{
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


void AThirdPersonCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !IsTargetLocked())
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
		hit = false;
		if (!IsEquipped) {
			IsEquipped = true;
			Equip();
		}

		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (canAttack) {
			// make the player face the direction of the input
			if (Controller != nullptr)
			{
				// Get the movement input vector
				FVector MoveDirection = GetLastMovementInputVector();
				if (MoveDirection.Size() < 0.1) {
					MoveDirection = GetActorForwardVector();
				}
				FRotator NewRotation = MoveDirection.Rotation();
				if (TargetLockComponent->GetIsLockedOn()) {
					NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLockComponent->GetTargetLocation());
				}
				NewRotation.Pitch = 0; // Ensure we only rotate around the Yaw axis
				SetActorRotation(NewRotation);
			}

			if (!GetMovementComponent()->IsFalling()) {
				canAttack = false;
				animInstance->Montage_Play(AttackMontage, 1.0f);

				OnAttackEndDelegate.BindUObject(this, &AThirdPersonCharacter::OnAttackMontageEnded);
				animInstance->Montage_SetEndDelegate(OnAttackEndDelegate, AttackMontage);

				FName section = FName(*FString::Printf(TEXT("Attack%d"), AttackCount + 1));
				if (CustomCharacterMovementComponent->IsSprinting()) {
					animInstance->Montage_JumpToSection("SprintAttack");
					CustomCharacterMovementComponent->StopSprint();
					AttackMultiplier = sprintAttackMultiplier;
				}
				else { 
					animInstance->Montage_JumpToSection(section); 
				}
				AttackCount = (AttackCount + 1) % (animInstance->GetCurrentActiveMontage()->GetNumSections()-1);
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


	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (bHeavyAttack)
	{
		if (!IsEquipped) {
			IsEquipped = true;
			Equip();
		}
		if (canAttack) {
			if (TargetLockComponent->GetIsLockedOn()) {
				FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLockComponent->GetTargetLocation());
				NewRotation.Pitch = 0; // Ensure we only rotate around the Yaw axis
				SetActorRotation(NewRotation);
			}
			canAttack = false;
			animInstance->Montage_Play(HeavyAttackMontage, 1.0f);
		}
	}
	else {
		if (animInstance->Montage_IsPlaying(HeavyAttackMontage)) {
			FString CurrentSectionName;
			FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(HeavyAttackMontage);
			if (MontageInstance)
			{
				CurrentSectionName = MontageInstance->GetCurrentSection().ToString();
			}
			if (CurrentSectionName.Equals("Mid")) {
				OnAttackEndDelegate.BindUObject(this, &AThirdPersonCharacter::OnAttackMontageEnded);
				animInstance->Montage_SetEndDelegate(OnAttackEndDelegate, HeavyAttackMontage);
				animInstance->Montage_Play(HeavyAttackMontage, 1.0f);
				animInstance->Montage_JumpToSection("End");
				AttackMultiplier = heavyAttackMultiplier;
			}
			else {
				animInstance->Montage_Stop(0.5f, HeavyAttackMontage);
				ResetAttack();
			}
		}
	}
}

bool AThirdPersonCharacter::IsPlayingMontage() const
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	return animInstance->Montage_IsPlaying(AttackMontage) || animInstance->Montage_IsPlaying(DrawMontage);
}

void AThirdPersonCharacter::Equip()
{
	if (WeaponMeshComponent)
	{
		FName SocketName = IsEquipped ? TEXT("katana3") : TEXT("katana1"); 
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	}
}


void AThirdPersonCharacter::AttackHitDetection()
{
	FVector StartLocation = WeaponMeshComponent->GetSocketLocation("Start");
	FVector EndLocation = WeaponMeshComponent->GetSocketLocation("End");

	float Radius = 20.0f;
	float HalfHeight = (EndLocation - StartLocation).Size() / 2;
	FQuat Rotation = WeaponMeshComponent->GetSocketQuaternion("End");

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
		// HitStop effect
		if (!hit) {
			hit = true;
			TriggerSlowMotion();
			//CameraShake
			if (CameraShake) {
				UGameplayStatics::GetPlayerController(this, 0)->ClientStartCameraShake(CameraShake);
			}
		}
		AActor* HitActor = HitResult.GetActor();

		float DamageAmount = WeaponDamage * AttackMultiplier;

		UE_LOG(LogTemp, Warning, TEXT("WeaponDamage: %f"), WeaponDamage);
		UE_LOG(LogTemp, Warning, TEXT("AttackMult: %f"), AttackMultiplier);
		
		IMeleeHitInterface* MeleeHitActor = Cast<IMeleeHitInterface>(HitActor);
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
		}
	}

	// Debug
	if(Debug) DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, HalfHeight, Radius, Rotation, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.0f);
}

bool AThirdPersonCharacter::IsTargetLocked()
{
	if (TargetLockComponent) {
		return TargetLockComponent->GetIsLockedOn();
	}
	return false;
}



void AThirdPersonCharacter::ResetTimeDilation()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void AThirdPersonCharacter::HandleTimeDilation(float value)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), value);
}

void AThirdPersonCharacter::TriggerSlowMotion()
{
	if (TimeDilationTimeline)
	{
		if (!TimeDilationTimeline->IsPlaying())
		{
			TimeDilationTimeline->PlayFromStart();
		}
	}
}

void AThirdPersonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (!canAttack) {
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		animInstance->Montage_Play(JumpAttackMontage, 1.0f);
		animInstance->Montage_JumpToSection("Land");
		ResetAttack();
		canMove = false;
		// Set delegate to reset canMove after landing
		OnJumpAttackEndDelegate.BindUObject(this, &AThirdPersonCharacter::OnJumpAttackMontageEnded);
		animInstance->Montage_SetEndDelegate(OnJumpAttackEndDelegate, JumpAttackMontage);

	}
}

void AThirdPersonCharacter::ResetAttack()
{
	canAttack = true;
	AttackCount = 0;
	AttackMultiplier = 1.0f;
	hit = false;
	UE_LOG(LogTemp, Warning, TEXT("Attack Reset"));
}

void AThirdPersonCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		ResetAttack();
	}
	else {
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (!animInstance->Montage_IsPlaying(AttackMontage)) {
			ResetAttack();
		}
	}
}

void AThirdPersonCharacter::OnJumpAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	SetCanMove();
}

void AThirdPersonCharacter::SetCanAttack()
{
	this->canAttack = true;
}

void AThirdPersonCharacter::SetCanMove()
{
	this->canMove = true;
}

bool AThirdPersonCharacter::GetIsEquipped()
{
	return IsEquipped;
}
