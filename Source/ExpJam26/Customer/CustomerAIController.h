// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "CustomerAIController.generated.h"

class UStateTreeAIComponent;
class ACustomerNPC;

// Non-dynamic single-cast delegates for StateTree tasks to bind to
DECLARE_DELEGATE(FCustomerInteractionStartedDelegate);
DECLARE_DELEGATE(FCustomerInteractionEndedDelegate);
DECLARE_DELEGATE(FCustomerWantsToGoToShopDelegate);
DECLARE_DELEGATE(FCustomerMoveCompletedDelegate);

/**
 *  AI Controller for customer NPCs.
 *  Bridges ACustomerNPC's dynamic multicast delegates to single-cast hooks
 *  that StateTree tasks can bind to via BindLambda.
 */
UCLASS(abstract)
class EXPJAM26_API ACustomerAIController : public AAIController
{
	GENERATED_BODY()

	/** Runs the behavior StateTree for this NPC */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStateTreeAIComponent* StateTreeAI;

protected:

	/** The NPC this controller currently possesses */
	TObjectPtr<ACustomerNPC> PossessedNPC;

public:

	/** Forwarded from ACustomerNPC::OnInteractionStarted; StateTree tasks bind here */
	FCustomerInteractionStartedDelegate OnCustomerInteractionStarted;

	/** Forwarded from ACustomerNPC::OnInteractionEnded; StateTree tasks bind here */
	FCustomerInteractionEndedDelegate OnCustomerInteractionEnded;

	/** Forwarded from ACustomerNPC::OnWantsToGoToShop; StateTree tasks bind here */
	FCustomerWantsToGoToShopDelegate OnCustomerWantsToGoToShop;

	/** Forwarded from ReceiveMoveCompleted; StateTree tasks bind here */
	FCustomerMoveCompletedDelegate OnCustomerMoveCompleted;

public:

	ACustomerAIController();

	/** Returns the currently possessed NPC */
	ACustomerNPC* GetPossessedNPC() const { return PossessedNPC; }

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:

	UFUNCTION()
	void HandleInteractionStarted();

	UFUNCTION()
	void HandleInteractionEnded();

	UFUNCTION()
	void HandleWantsToGoToShop();

	UFUNCTION()
	void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
};
