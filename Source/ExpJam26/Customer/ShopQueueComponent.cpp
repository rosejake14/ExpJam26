// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShopQueueComponent.h"
#include "CustomerNPC.h"
#include "GameFramework/Actor.h"

UShopQueueComponent::UShopQueueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UShopQueueComponent::JoinQueue(ACustomerNPC* NPC, FVector& OutSlotPosition)
{
	PurgeStaleEntries();

	if (Queue.Num() >= MaxQueueSize)
	{
		return false;
	}

	const int32 SlotIndex = Queue.Num();
	Queue.Add(TWeakObjectPtr<ACustomerNPC>(NPC));
	OutSlotPosition = CalculateSlotPosition(SlotIndex);
	return true;
}

void UShopQueueComponent::LeaveQueue(ACustomerNPC* NPC)
{
	PurgeStaleEntries();

	const int32 RemovedIndex = Queue.IndexOfByPredicate([NPC](const TWeakObjectPtr<ACustomerNPC>& Ptr)
	{
		return Ptr.Get() == NPC;
	});

	if (RemovedIndex == INDEX_NONE)
	{
		return;
	}

	Queue.RemoveAt(RemovedIndex);

	// advance every NPC that was behind the removed one
	for (int32 i = RemovedIndex; i < Queue.Num(); ++i)
	{
		if (ACustomerNPC* AdvancingNPC = Queue[i].Get())
		{
			AdvancingNPC->AdvanceToQueuePosition(CalculateSlotPosition(i));
		}
	}
}

bool UShopQueueComponent::IsAtFrontOfQueue(const ACustomerNPC* NPC) const
{
	return !Queue.IsEmpty() && Queue[0].Get() == NPC;
}

FVector UShopQueueComponent::CalculateSlotPosition(int32 SlotIndex) const
{
	const AActor* Owner = GetOwner();

	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	const FVector WorldDir = Owner->GetActorRotation().RotateVector(QueueDirectionLocal).GetSafeNormal();
	return Owner->GetActorLocation() + WorldDir * (FrontOfQueueDistance + SlotIndex * SlotSpacing);
}

void UShopQueueComponent::PurgeStaleEntries()
{
	Queue.RemoveAll([](const TWeakObjectPtr<ACustomerNPC>& Ptr)
	{
		return !Ptr.IsValid();
	});
}
