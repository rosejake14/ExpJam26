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
#include "Inventory/CraftingRecipe.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/MoneyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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

	// order arrow — mesh and material assigned in Blueprint; hidden until player holds the requested item
	OrderArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrderArrow"));
	OrderArrow->SetupAttachment(RootComponent);
	OrderArrow->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));
	OrderArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OrderArrow->SetHiddenInGame(true);
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

	// bypass APawn::EnableInput's "own controller only" restriction by calling AActor's version directly
	AActor::EnableInput(PlayerController);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ACustomerNPC::Interact);
		}
		if (DeclineAction)
		{
			EIC->BindAction(DeclineAction, ETriggerEvent::Started, this, &ACustomerNPC::DeclineRequest);
		}
	}

	// show the "Press E to talk" prompt on the player's HUD
	if (OtherActor->Implements<UInteractionPromptHandler>())
	{
		IInteractionPromptHandler::Execute_ShowInteractionPrompt(OtherActor, this,
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

	if (!Player || FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) > MaxDistance)
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

	// E on the recipe prompt = accept
	if (bShowingRecipePrompt)
	{
		AcceptRequest();
		return;
	}

	// advance to the next dialogue line
	++CurrentDialogueIndex;

	if (CurrentDialogueIndex < DialogueLines.Num())
	{
		BP_OnDialogueLineChanged(DialogueLines[CurrentDialogueIndex]);
	}
	else if (ActiveRequest)
	{
		// finished all dialogue lines — offer the recipe request
		bShowingRecipePrompt = true;
		BP_OnShowRecipePrompt(ActiveRequest);
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

	// if waiting for a delivery, skip dialogue and go straight to the check
	if (bHasActiveRequest)
	{
		TryDeliverRequest();
		return;
	}

	// pick a random recipe to offer at the end of this conversation
	if (RequestableRecipes.Num() > 0)
	{
		ActiveRequest = RequestableRecipes[FMath::RandRange(0, RequestableRecipes.Num() - 1)];
	}

	// show the floating dialogue bubble
	if (DialogueWidget)
	{
		DialogueWidget->SetHiddenInGame(false);
	}

	// create the HUD widget first, then push the first line into it
	BP_OnInteractionBegan(Player);

	if (DialogueLines.Num() > 0)
	{
		BP_OnDialogueLineChanged(DialogueLines[0]);
	}

	OnInteractionStarted.Broadcast();
}

void ACustomerNPC::EndInteraction()
{
	bIsBeingInteracted = false;
	CurrentDialogueIndex = 0;

	// if the player walked away while the prompt was showing, treat it as a decline
	if (bShowingRecipePrompt)
	{
		bShowingRecipePrompt = false;
		ActiveRequest = nullptr;
		BP_OnRecipeRequestDeclined();
	}

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
			IInteractionPromptHandler::Execute_HideInteractionPrompt(Player, this);
		}

		if (APawn* Pawn = Cast<APawn>(Player))
		{
			if (APlayerController* PC = Pawn->GetController<APlayerController>())
			{
				if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
				{
					EIC->ClearActionBindings();
				}

				AActor::DisableInput(PC);
			}
		}
	}

	InteractingPlayer = nullptr;
}

void ACustomerNPC::AcceptRequest()
{
	bShowingRecipePrompt = false;
	bHasActiveRequest = true;

	// send the NPC straight to the shop to wait for delivery
	bWantsToGoToShop = true;
	OnWantsToGoToShop.Broadcast();

	BP_OnRecipeRequestAccepted(ActiveRequest);
	EndInteraction();
}

void ACustomerNPC::DeclineRequest()
{
	if (!bIsBeingInteracted || !bShowingRecipePrompt)
	{
		return;
	}

	bShowingRecipePrompt = false;
	ActiveRequest = nullptr;
	BP_OnRecipeRequestDeclined();
	EndInteraction();
}

void ACustomerNPC::TryDeliverRequest()
{
	AActor* Player = InteractingPlayer.Get();
	if (!Player || !ActiveRequest)
	{
		EndInteraction();
		return;
	}

	UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		EndInteraction();
		return;
	}

	const FItemStack& Required = ActiveRequest->Result;
	if (Inventory->GetItemCount(Required.Item) >= Required.Quantity)
	{
		Inventory->RemoveItem(Required.Item, Required.Quantity);
		bHasActiveRequest = false;
		ActiveRequest = nullptr;
		if (OrderArrow) { OrderArrow->SetHiddenInGame(true); }

		if (UMoneyComponent* Money = Player->FindComponentByClass<UMoneyComponent>())
		{
			Money->AddMoney(FMath::RandRange(MinOrderReward, MaxOrderReward));
		}

		BP_OnRecipeRequestCompleted();
	}
	else
	{
		BP_OnDeliveryFailed();
	}

	EndInteraction();
}

void ACustomerNPC::RefreshOrderArrow()
{
	if (!OrderArrow) { return; }

	if (!bHasActiveRequest || !ActiveRequest)
	{
		OrderArrow->SetHiddenInGame(true);
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		OrderArrow->SetHiddenInGame(true);
		return;
	}

	UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		OrderArrow->SetHiddenInGame(true);
		return;
	}

	const FItemStack& Result = ActiveRequest->Result;
	const bool bPlayerHasItem = Result.Item && Inventory->GetItemCount(Result.Item) >= Result.Quantity;
	OrderArrow->SetHiddenInGame(!bPlayerHasItem);
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
	return RoamCenter.IsZero() ? SpawnLocation : RoamCenter;
}

void ACustomerNPC::RandomizeNextShopTripCycles()
{
	TargetRoamCycles = RoamCyclesBeforeShop + FMath::RandRange(-RoamCyclesVariance, RoamCyclesVariance);
	TargetRoamCycles = FMath::Max(1, TargetRoamCycles);
}
