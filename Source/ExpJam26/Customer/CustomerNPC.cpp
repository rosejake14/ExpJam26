// Copyright Epic Games, Inc. All Rights Reserved.


#include "CustomerNPC.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "InteractionPromptHandler.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACustomerNPC::ACustomerNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	// interaction detection sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRadius);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldStatic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->bFillCollisionUnderneathForNavmesh = true;
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACustomerNPC::OnInteractionSphereOverlap);

	// floating dialogue widget, positioned above the character's head
	DialogueWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DialogueWidget"));
	DialogueWidget->SetupAttachment(GetMesh());
	DialogueWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	DialogueWidget->SetWidgetSpace(EWidgetSpace::World);
	DialogueWidget->SetDrawAtDesiredSize(true);
	DialogueWidget->SetHiddenInGame(true);
}

void ACustomerNPC::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	RandomizeNextShopTripCycles();
}

void ACustomerNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(InteractionRangeCheckTimer);
}

void ACustomerNPC::OnInteractionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// only respond to the first player in range
	if (InteractingPlayer.IsValid())
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	APlayerController* PlayerController = Pawn->GetController<APlayerController>();
	if (!PlayerController)
	{
		return;
	}

	InteractingPlayer = OtherActor;

	// enable input on this NPC actor so it can receive the interact action
	EnableInput(PlayerController);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ACustomerNPC::Interact);
		}
	}

	// show the "Press E to talk" prompt on the player's HUD
	if (OtherActor->Implements<UInteractionPromptHandler>())
	{
		IInteractionPromptHandler::Execute_ShowInteractionPrompt(OtherActor,
			NSLOCTEXT("CustomerNPC", "TalkPrompt", "Press E to talk"));
	}

	// poll distance so we can clean up if the player walks away
	GetWorld()->GetTimerManager().SetTimer(InteractionRangeCheckTimer, this,
		&ACustomerNPC::CheckInteractionRange, 0.2f, true);
}

void ACustomerNPC::CheckInteractionRange()
{
	const float MaxDistance = InteractionSphere->GetScaledSphereRadius() + 50.0f;
	AActor* Player = InteractingPlayer.Get();

	if (!Player || FVector::Dist(Player->GetActorLocation(), GetActorLocation()) > MaxDistance)
	{
		if (bIsBeingInteracted)
		{
			EndInteraction();
		}
		else
		{
			StopInteractingPlayer();
		}
	}
}

void ACustomerNPC::Interact()
{
	if (!bIsBeingInteracted)
	{
		if (AActor* Player = InteractingPlayer.Get())
		{
			BeginInteraction(Player);
		}
		return;
	}

	// advance to the next dialogue line
	++CurrentDialogueIndex;

	if (CurrentDialogueIndex < DialogueLines.Num())
	{
		BP_OnDialogueLineChanged(DialogueLines[CurrentDialogueIndex]);
	}
	else
	{
		EndInteraction();
	}
}

void ACustomerNPC::BeginInteraction(AActor* Player)
{
	bIsBeingInteracted = true;

	// stop movement and face the player
	if (AAIController* AIController = GetController<AAIController>())
	{
		AIController->StopMovement();
		AIController->SetFocus(Player);
	}

	// show the floating dialogue bubble and display the first line
	if (DialogueWidget)
	{
		DialogueWidget->SetHiddenInGame(false);
	}

	if (DialogueLines.Num() > 0)
	{
		BP_OnDialogueLineChanged(DialogueLines[0]);
	}

	// notify Blueprint (to show HUD widget) and StateTree (to block movement)
	BP_OnInteractionBegan(Player);
	OnInteractionStarted.Broadcast();
}

void ACustomerNPC::EndInteraction()
{
	bIsBeingInteracted = false;
	CurrentDialogueIndex = 0;

	// clear NPC focus so it stops staring at the player
	if (AAIController* AIController = GetController<AAIController>())
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	HideDialogueWidget();
	BP_OnInteractionEnded();
	OnInteractionEnded.Broadcast();

	StopInteractingPlayer();
}

void ACustomerNPC::HideDialogueWidget()
{
	if (DialogueWidget)
	{
		DialogueWidget->SetHiddenInGame(true);
	}
}

void ACustomerNPC::StopInteractingPlayer()
{
	GetWorld()->GetTimerManager().ClearTimer(InteractionRangeCheckTimer);

	if (AActor* Player = InteractingPlayer.Get())
	{
		if (Player->Implements<UInteractionPromptHandler>())
		{
			IInteractionPromptHandler::Execute_HideInteractionPrompt(Player);
		}

		if (APawn* Pawn = Cast<APawn>(Player))
		{
			if (APlayerController* PC = Pawn->GetController<APlayerController>())
			{
				if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
				{
					EIC->ClearActionBindings();
				}

				DisableInput(PC);
			}
		}
	}

	InteractingPlayer = nullptr;
}

void ACustomerNPC::AdvanceToQueuePosition(FVector NewPosition)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		AIController->MoveToLocation(NewPosition);
	}
}

void ACustomerNPC::OnRoamCycleCompleted()
{
	++RoamCyclesCompleted;

	if (RoamCyclesCompleted >= TargetRoamCycles)
	{
		RoamCyclesCompleted = 0;
		bWantsToGoToShop = true;
		OnWantsToGoToShop.Broadcast();
		RandomizeNextShopTripCycles();
	}
}

FVector ACustomerNPC::GetEffectiveRoamCenter() const
{
	return RoamCenter.IsNearlyZero() ? SpawnLocation : RoamCenter;
}

void ACustomerNPC::RandomizeNextShopTripCycles()
{
	TargetRoamCycles = RoamCyclesBeforeShop + FMath::RandRange(-RoamCyclesVariance, RoamCyclesVariance);
	TargetRoamCycles = FMath::Max(1, TargetRoamCycles);
}
