// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CustomerNPC.generated.h"

class USphereComponent;
class UWidgetComponent;
class UInputAction;

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

protected:

	/** Radius of the player-detection sphere */
	UPROPERTY(EditAnywhere, Category="Customer|Interaction")
	float InteractionRadius = 200.0f;

	/** Input action bound to talking (should reference IA_Interact) */
	UPROPERTY(EditDefaultsOnly, Category="Customer|Interaction")
	TObjectPtr<UInputAction> InteractAction;

	/** Lines of dialogue cycled through when the player presses E */
	UPROPERTY(EditAnywhere, Category="Customer|Dialogue")
	TArray<FText> DialogueLines;

public:

	/** Maximum distance the NPC will roam from its center point */
	UPROPERTY(EditAnywhere, Category="Customer|Roam")
	float RoamRadius = 1500.0f;

	/** Explicit roam center; if zero-vector, captured from spawn location at BeginPlay */
	UPROPERTY(EditAnywhere, Category="Customer|Roam")
	FVector RoamCenter = FVector::ZeroVector;

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

private:

	/** Clears the input binding, hides the prompt, and stops the range-check timer */
	void StopInteractingPlayer();

	/** Randomizes the number of roam cycles before the next shop visit */
	void RandomizeNextShopTripCycles();
};
