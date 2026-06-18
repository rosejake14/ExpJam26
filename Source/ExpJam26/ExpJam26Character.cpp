// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExpJam26Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Inventory/InventoryComponent.h"
#include "ExpJam26.h"

AExpJam26Character::AExpJam26Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AExpJam26Character::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AExpJam26Character::DoStartSprint()
{
	bSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AExpJam26Character::DoEndSprint()
{
	bSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AExpJam26Character::DoToggleDestroyMode()
{
	bDestroyModeActive = !bDestroyModeActive;
	BP_OnDestroyModeChanged(bDestroyModeActive);
}

void AExpJam26Character::DoDestroySlot(int32 SlotIndex)
{
	if (!bDestroyModeActive)
	{
		return;
	}

	UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	Inventory->RemoveSlot(SlotIndex);

	bDestroyModeActive = false;
	BP_OnDestroyModeChanged(false);
}

void AExpJam26Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AExpJam26Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AExpJam26Character::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExpJam26Character::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExpJam26Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AExpJam26Character::LookInput);

		// Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AExpJam26Character::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AExpJam26Character::DoEndSprint);
		}

		// Destroy mode
		if (DestroyModeAction)
		{
			EnhancedInputComponent->BindAction(DestroyModeAction, ETriggerEvent::Started, this, &AExpJam26Character::DoToggleDestroyMode);
		}
		if (DestroySlot1Action)
		{
			EnhancedInputComponent->BindAction(DestroySlot1Action, ETriggerEvent::Started, this, &AExpJam26Character::DoDestroySlot1);
		}
		if (DestroySlot2Action)
		{
			EnhancedInputComponent->BindAction(DestroySlot2Action, ETriggerEvent::Started, this, &AExpJam26Character::DoDestroySlot2);
		}
		if (DestroySlot3Action)
		{
			EnhancedInputComponent->BindAction(DestroySlot3Action, ETriggerEvent::Started, this, &AExpJam26Character::DoDestroySlot3);
		}
	}
	else
	{
		UE_LOG(LogExpJam26, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AExpJam26Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AExpJam26Character::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AExpJam26Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AExpJam26Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AExpJam26Character::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AExpJam26Character::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
