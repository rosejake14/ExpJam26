// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CustomerNPC.generated.h"

class USphereComponent;
class UWidgetComponent;
class UStaticMeshComponent;
class UInputAction;
class UCraftingRecipe;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCustomerNPCInteractionChangedDelegate);

/**
 *  A friendly AI customer NPC that roams an area, interacts with the player on request,
 *  and periodically walks to the player's shop to join a queue.
 */
UCLASS(abstract)
class EXPJAM26_API ACustomerNPC : public ACharacter
{
	GENERATED_BODY()

	/** Sphere that detects the player and triggers the interaction prompt */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* InteractionSphere;

	/** World-space widget shown above the NPC's head during dialogue */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UWidgetComponent* DialogueWidget;

	/** Arrow mesh shown above the NPC (through walls) when the player is holding the requested item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* OrderArrow;

protected:

	/** Radius of the player-detection sphere */
	UPROPERTY(EditAnywhere, Category="Customer|Interaction")
	float InteractionRadius = 200.0f;

	/** Input action bound to talking (should reference IA_Interact) */
	UPROPERTY(EditDefaultsOnly, Category="Customer|Interaction")
	TObjectPtr<UInputAction> InteractAction;

	/** Input action bound to declining a recipe request (e.g. IA_Decline / F key) */
	UPROPERTY(EditDefaultsOnly, Category="Customer|Interaction")
	TObjectPtr<UInputAction> DeclineAction;

	/** Lines of dialogue cycled through when the player presses E */
	UPROPERTY(EditAnywhere, Category="Customer|Dialogue")
	TArray<FText> DialogueLines;

	/** Pool of recipes this NPC can randomly request from the player */
	UPROPERTY(EditAnywhere, Category="Customer|Request")
	TArray<TObjectPtr<UCraftingRecipe>> RequestableRecipes;

public:

	/** Maximum distance the NPC will roam from where it was placed in the level */
	UPROPERTY(EditAnywhere, Category="Customer|Roam")
	float RoamRadius = 1500.0f;

protected:

	/** Average number of roam cycles between shop visits */
	UPROPERTY(EditAnywhere, Category="Customer|Shop", meta=(ClampMin=1))
	int32 RoamCyclesBeforeShop = 5;

	/** Random variance applied to RoamCyclesBeforeShop (+/-) */
	UPROPERTY(EditAnywhere, Category="Customer|Shop", meta=(ClampMin=0))
	int32 RoamCyclesVariance = 2;

public:

	/** Minimum time the NPC spends at the front of the queue before leaving */
	UPROPERTY(EditAnywhere, Category="Customer|Shop")
	float MinShopWaitTime = 10.0f;

	/** Maximum time the NPC spends at the front of the queue before leaving */
	UPROPERTY(EditAnywhere, Category="Customer|Shop")
	float MaxShopWaitTime = 30.0f;

	/** Actor with a UShopQueueComponent that this NPC will walk to as a customer */
	UPROPERTY(EditAnywhere, Category="Customer|Shop")
	TObjectPtr<AActor> ShopActor;

public:

	/** True while the player is actively in a dialogue with this NPC */
	bool bIsBeingInteracted = false;

	/** True when this NPC has decided to go to the shop on the next roam cycle */
	bool bWantsToGoToShop = false;

	/** True while this NPC is waiting for the player to deliver a crafted item */
	UPROPERTY(BlueprintReadOnly, Category="Customer|Request")
	bool bHasActiveRequest = false;

	/** The recipe this NPC is currently requesting; null when no request is active */
	UPROPERTY(BlueprintReadOnly, Category="Customer|Request")
	TObjectPtr<UCraftingRecipe> ActiveRequest;

	/** Broadcast when the player begins a dialogue with this NPC */
	UPROPERTY(BlueprintAssignable)
	FCustomerNPCInteractionChangedDelegate OnInteractionStarted;

	/** Broadcast when the dialogue with the player ends */
	UPROPERTY(BlueprintAssignable)
	FCustomerNPCInteractionChangedDelegate OnInteractionEnded;

	/** Broadcast when this NPC decides it wants to visit the shop */
	UPROPERTY(BlueprintAssignable)
	FCustomerNPCInteractionChangedDelegate OnWantsToGoToShop;

private:

	/** Pawn currently in interaction range */
	TWeakObjectPtr<AActor> InteractingPlayer;

	/** Index of the dialogue line currently being displayed */
	int32 CurrentDialogueIndex = 0;

	/** Number of roam cycles completed since the last shop visit */
	int32 RoamCyclesCompleted = 0;

	/** Roam cycle threshold randomized at start and after each shop visit */
	int32 TargetRoamCycles = 5;

	/** Actor location captured at BeginPlay, used as default RoamCenter */
	FVector SpawnLocation = FVector::ZeroVector;

	/** Polls interaction range every 0.2s, same pattern as AItemPickup */
	FTimerHandle InteractionRangeCheckTimer;

public:

	ACustomerNPC();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called when an actor enters the interaction sphere */
	UFUNCTION()
	void OnInteractionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Checks every 0.2s whether the interacting player is still in range */
	void CheckInteractionRange();

public:

	/** Called when the player presses E — starts or advances the current dialogue */
	void Interact();

	/** Starts interaction: stops NPC movement, shows widgets, notifies StateTree */
	void BeginInteraction(AActor* Player);

	/** Ends interaction: hides widgets, resumes normal AI, notifies StateTree */
	void EndInteraction();

	/** Called by UShopQueueComponent when the queue shifts; issues a fresh MoveToLocation */
	void AdvanceToQueuePosition(FVector NewPosition);

	/**
	 * Shows the OrderArrow if the player currently holds the requested crafted item, hides it otherwise.
	 * Bind this to the player inventory's OnInventoryUpdated in Blueprint after accepting a request.
	 */
	UFUNCTION(BlueprintCallable, Category="Customer|Request")
	void RefreshOrderArrow();

	/** Increments the roam cycle counter; triggers shop visit when threshold is reached */
	void OnRoamCycleCompleted();

	/** Hides the floating dialogue widget */
	void HideDialogueWidget();

	/** Returns the effective roam center (RoamCenter if set, otherwise SpawnLocation) */
	FVector GetEffectiveRoamCenter() const;

protected:

	/** Called by BeginInteraction; override in Blueprint to show the screen HUD widget */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Dialogue")
	void BP_OnInteractionBegan(AActor* PlayerActor);

	/** Called by EndInteraction; override in Blueprint to hide the screen HUD widget */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Dialogue")
	void BP_OnInteractionEnded();

	/** Called when the active dialogue line changes; override in Blueprint to update widget text */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Dialogue")
	void BP_OnDialogueLineChanged(const FText& NewLine);

	/**
	 * Called after the last dialogue line when this NPC has a recipe to request.
	 * Override in Blueprint to show the accept/decline choice UI.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Request")
	void BP_OnShowRecipePrompt(UCraftingRecipe* Recipe);

	/**
	 * Called when the player presses E to accept a recipe request.
	 * Override in Blueprint to spawn the order-tracker HUD widget and close the dialogue panel.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Request")
	void BP_OnRecipeRequestAccepted(UCraftingRecipe* Recipe);

	/** Called when the player presses the decline key. Override in Blueprint to clean up the prompt UI. */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Request")
	void BP_OnRecipeRequestDeclined();

	/**
	 * Called when the player returns with the requested crafted item and it is successfully removed.
	 * Override in Blueprint to dismiss the order-tracker HUD and play a reward effect.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Request")
	void BP_OnRecipeRequestCompleted();

	/**
	 * Called when the player interacts while a request is active but does not yet have the item.
	 * Override in Blueprint to show a brief "not ready yet" response.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Customer|Request")
	void BP_OnDeliveryFailed();

private:

	/** Clears the input binding, hides the prompt, and stops the range-check timer */
	void StopInteractingPlayer();

	/** Randomizes the number of roam cycles before the next shop visit */
	void RandomizeNextShopTripCycles();

	/** True when the NPC has shown the recipe prompt and is waiting for the player to accept/decline */
	bool bShowingRecipePrompt = false;

	/** Player accepted the recipe prompt — marks active request and ends dialogue */
	void AcceptRequest();

	/** Player pressed the decline key — clears the recipe choice and ends dialogue */
	void DeclineRequest();

	/**
	 * Checks if the player's inventory contains the requested crafted item.
	 * On success removes it and fires BP_OnRecipeRequestCompleted.
	 * On failure fires BP_OnDeliveryFailed.
	 */
	void TryDeliverRequest();
};
