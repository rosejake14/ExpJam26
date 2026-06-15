// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InventoryComponent.h"
#include "InteractionPromptHandler.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);

	CollisionSphere->SetSphereRadius(64.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldStatic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->bFillCollisionUnderneathForNavmesh = true;

	// subscribe to the collision overlap on the sphere
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnOverlap);

	// create the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CollisionSphere);

	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void AItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Item)
	{
		// set the mesh from the item definition
		Mesh->SetStaticMesh(Item->PickupMesh.LoadSynchronous());
	}
}

void AItemPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);

	// clear the interaction range check timer
	GetWorld()->GetTimerManager().ClearTimer(InteractionRangeCheckTimer);
}

void AItemPickup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Item)
	{
		return;
	}

	// have we collided against something with an inventory?
	UInventoryComponent* Inventory = OtherActor ? OtherActor->FindComponentByClass<UInventoryComponent>() : nullptr;

	if (!Inventory)
	{
		return;
	}

	if (bRequiresInteraction)
	{
		// track the actor so Interact() knows who to give the item to
		InteractingActor = OtherActor;

		if (APawn* Pawn = Cast<APawn>(OtherActor))
		{
			if (APlayerController* PlayerController = Pawn->GetController<APlayerController>())
			{
				EnableInput(PlayerController);

				if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
				{
					if (InteractAction)
					{
						EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AItemPickup::Interact);
					}
				}
			}
		}

		// show a "press E to pick up" prompt on the interactor's HUD
		if (OtherActor->Implements<UInteractionPromptHandler>())
		{
			const FText PromptText = FText::Format(NSLOCTEXT("ItemPickup", "InteractionPrompt", "Press E to pick up {0}"), Item->DisplayName);
			IInteractionPromptHandler::Execute_ShowInteractionPrompt(OtherActor, PromptText);
		}

		// start polling for the interactor leaving range, since OnComponentEndOverlap isn't reliable here
		GetWorld()->GetTimerManager().SetTimer(InteractionRangeCheckTimer, this, &AItemPickup::CheckInteractionRange, 0.2f, true);

		return;
	}

	GiveItem(Inventory);
}

void AItemPickup::CheckInteractionRange()
{
	AActor* Interactor = InteractingActor.Get();

	if (!Interactor || FVector::Dist(Interactor->GetActorLocation(), GetActorLocation()) > CollisionSphere->GetScaledSphereRadius())
	{
		StopInteracting();
	}
}

void AItemPickup::Interact()
{
	AActor* Interactor = InteractingActor.Get();

	if (!Interactor)
	{
		return;
	}

	UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();

	if (!Inventory)
	{
		return;
	}

	GiveItem(Inventory);
}

void AItemPickup::GiveItem(UInventoryComponent* Inventory)
{
	const int32 Leftover = Inventory->AddItem(Item, Quantity);

	// did we manage to give at least one item?
	if (Leftover < Quantity)
	{
		if (bRequiresInteraction)
		{
			StopInteracting();
		}

		if (RespawnTime > 0.0f)
		{
			// hide this mesh
			SetActorHiddenInGame(true);

			// disable collision
			SetActorEnableCollision(false);

			// schedule the respawn
			GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AItemPickup::RespawnPickup, RespawnTime, false);
		}
		else
		{
			// one-shot pickup, remove it from the world
			Destroy();
		}
	}
}

void AItemPickup::StopInteracting()
{
	GetWorld()->GetTimerManager().ClearTimer(InteractionRangeCheckTimer);

	if (AActor* Interactor = InteractingActor.Get())
	{
		if (Interactor->Implements<UInteractionPromptHandler>())
		{
			IInteractionPromptHandler::Execute_HideInteractionPrompt(Interactor);
		}

		if (APawn* Pawn = Cast<APawn>(Interactor))
		{
			if (APlayerController* PlayerController = Pawn->GetController<APlayerController>())
			{
				if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
				{
					EnhancedInputComponent->ClearActionBindings();
				}

				DisableInput(PlayerController);
			}
		}
	}

	InteractingActor = nullptr;
}

void AItemPickup::RespawnPickup()
{
	// unhide this pickup
	SetActorHiddenInGame(false);

	// call the BP handler
	BP_OnRespawn();
}

void AItemPickup::FinishRespawn()
{
	// enable collision
	SetActorEnableCollision(true);
}
