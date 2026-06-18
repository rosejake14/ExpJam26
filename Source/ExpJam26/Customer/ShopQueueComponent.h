// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShopQueueComponent.generated.h"

class ACustomerNPC;

/**
 *  Add to any Actor to turn it into a shop with a customer queue.
 *  Calculates world-space queue slot positions and notifies NPCs when they should advance.
 */
UCLASS(ClassGroup=(Customer), meta=(BlueprintSpawnableComponent))
class EXPJAM26_API UShopQueueComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	/** Distance from the shop actor origin to the front-of-queue slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop Queue")
	float FrontOfQueueDistance = 150.0f;

	/** Gap between successive queue slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop Queue")
	float SlotSpacing = 90.0f;

	/** Queue extends in this direction relative to the shop actor's local space.
	 *  Default (-1,0,0) places the queue in front of an actor facing +X. */
	UPROPERTY(EditAnywhere, Category="Shop Queue")
	FVector QueueDirectionLocal = FVector(-1.0f, 0.0f, 0.0f);

private:

	/** Ordered list of queued NPCs; index 0 is the customer being served */
	TArray<TWeakObjectPtr<ACustomerNPC>> Queue;

public:

	UShopQueueComponent();

	/** Adds the NPC to the back of the queue and returns the world position it should walk to */
	FVector JoinQueue(ACustomerNPC* NPC);

	/** Removes the NPC from the queue. Remaining NPCs behind it are shifted forward. */
	void LeaveQueue(ACustomerNPC* NPC);

	/** Returns true if the NPC is currently at the front of the queue */
	bool IsAtFrontOfQueue(const ACustomerNPC* NPC) const;

	/** Returns the current number of customers in the queue */
	int32 GetQueueLength() const { return Queue.Num(); }

private:

	/** Calculates the world-space position for a given queue slot index */
	FVector CalculateSlotPosition(int32 SlotIndex) const;

	/** Removes any entries pointing to destroyed NPCs */
	void PurgeStaleEntries();
};
