// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemDefinition.h"
#include "ItemPickup.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UInputAction;
class UInventoryComponent;

/**
 *  A world pickup that grants an item stack to any inventory that overlaps it.
 */
UCLASS(abstract)
class EXPJAM26_API AItemPickup : public AActor
{
	GENERATED_BODY()

	/** Collision sphere that detects overlapping inventory owners */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionSphere;

	/** Pickup mesh. Set from the item's PickupMesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** The item granted by this pickup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	TObjectPtr<UItemDefinition> Item;

	/** How many of Item to grant */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup", meta = (ClampMin = 1))
	int32 Quantity = 1;

	/** Time to wait before respawning this pickup after it's picked up. 0 means it won't respawn. */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 0.0f;

	/** If true, an overlapping actor must press InteractAction to collect this pickup, instead of collecting it automatically on overlap */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	bool bRequiresInteraction = false;

	/** Input action that triggers Interact() while bRequiresInteraction is true and an actor is in range */
	UPROPERTY(EditDefaultsOnly, Category="Pickup", meta = (EditCondition = "bRequiresInteraction"))
	TObjectPtr<UInputAction> InteractAction;

	/** The actor currently in range and able to call Interact, if bRequiresInteraction is true */
	TWeakObjectPtr<AActor> InteractingActor;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;

public:

	/** Constructor */
	AItemPickup();

	/** Gives this pickup's item to whichever actor is currently in range, if bRequiresInteraction is true */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void Interact();

protected:

	/** Native construction script */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Handles collision overlap */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Handles the end of a collision overlap */
	UFUNCTION()
	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Attempts to add Item to Inventory, then handles respawn/destruction if any were added */
	void GiveItem(UInventoryComponent* Inventory);

	/** Stops tracking InteractingActor, unbinds its interact input, and notifies Blueprint that this pickup is no longer interactable */
	void StopInteracting();

	/** Called when it's time to respawn this pickup */
	void RespawnPickup();

	/** Passes control to Blueprint to animate the pickup respawn. Should end by calling FinishRespawn */
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** Enables this pickup after respawning */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void FinishRespawn();

	/** Passes control to Blueprint when an actor comes into range (true) or leaves/collects (false). Only called if bRequiresInteraction is true */
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta = (DisplayName = "On Interactable Changed"))
	void BP_OnInteractableChanged(bool bCanInteract);
};
