// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// allocate the fixed slot grid
	Slots.SetNum(FMath::Max(NumSlots, 0));

	// grant any starting items
	for (const FItemStack& StartingItem : StartingItems)
	{
		if (StartingItem.Item && StartingItem.Quantity > 0)
		{
			AddItem(StartingItem.Item, StartingItem.Quantity);
		}
	}
}

int32 UInventoryComponent::AddItem(UItemDefinition* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return Quantity;
	}

	int32 Remaining = Quantity;

	// first, top up any existing stacks of this item
	for (FItemStack& Slot : Slots)
	{
		if (Remaining <= 0)
		{
			break;
		}

		if (Slot.Item == Item && Slot.Quantity < Item->MaxStackSize)
		{
			const int32 SpaceInSlot = Item->MaxStackSize - Slot.Quantity;
			const int32 AmountToAdd = FMath::Min(SpaceInSlot, Remaining);

			Slot.Quantity += AmountToAdd;
			Remaining -= AmountToAdd;
		}
	}

	// then fill any empty slots with the remainder
	for (FItemStack& Slot : Slots)
	{
		if (Remaining <= 0)
		{
			break;
		}

		if (Slot.IsEmpty())
		{
			const int32 AmountToAdd = FMath::Min(Item->MaxStackSize, Remaining);

			Slot.Item = Item;
			Slot.Quantity = AmountToAdd;
			Remaining -= AmountToAdd;
		}
	}

	if (Remaining != Quantity)
	{
		BroadcastUpdate();
	}

	return Remaining;
}

bool UInventoryComponent::RemoveItem(UItemDefinition* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0 || GetItemCount(Item) < Quantity)
	{
		return false;
	}

	int32 Remaining = Quantity;

	for (FItemStack& Slot : Slots)
	{
		if (Remaining <= 0)
		{
			break;
		}

		if (Slot.Item == Item)
		{
			const int32 AmountToRemove = FMath::Min(Slot.Quantity, Remaining);

			Slot.Quantity -= AmountToRemove;
			Remaining -= AmountToRemove;

			if (Slot.Quantity <= 0)
			{
				Slot = FItemStack();
			}
		}
	}

	BroadcastUpdate();

	return true;
}

bool UInventoryComponent::RemoveSlot(int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		return false;
	}

	Slots[SlotIndex] = FItemStack();
	BroadcastUpdate();
	return true;
}

int32 UInventoryComponent::GetItemCount(UItemDefinition* Item) const
{
	int32 Total = 0;

	for (const FItemStack& Slot : Slots)
	{
		if (Slot.Item == Item)
		{
			Total += Slot.Quantity;
		}
	}

	return Total;
}

FItemStack UInventoryComponent::GetSlot(int32 SlotIndex) const
{
	if (IsValidSlotIndex(SlotIndex))
	{
		return Slots[SlotIndex];
	}

	return FItemStack();
}

int32 UInventoryComponent::GetNumSlots() const
{
	return Slots.Num();
}

bool UInventoryComponent::MoveSlot(int32 SourceSlot, UInventoryComponent* TargetInventory, int32 TargetSlot)
{
	if (!TargetInventory || !IsValidSlotIndex(SourceSlot) || !TargetInventory->IsValidSlotIndex(TargetSlot))
	{
		return false;
	}

	if (this == TargetInventory && SourceSlot == TargetSlot)
	{
		return false;
	}

	FItemStack& SourceStack = Slots[SourceSlot];
	FItemStack& TargetStack = TargetInventory->Slots[TargetSlot];

	if (SourceStack.IsEmpty())
	{
		return false;
	}

	if (TargetStack.IsEmpty())
	{
		// move the stack into the empty slot
		TargetStack = SourceStack;
		SourceStack = FItemStack();
	}
	else if (TargetStack.Item == SourceStack.Item)
	{
		// merge as much of the source stack into the target stack as will fit
		const int32 SpaceInTarget = TargetStack.Item->MaxStackSize - TargetStack.Quantity;
		const int32 AmountToMove = FMath::Min(SpaceInTarget, SourceStack.Quantity);

		TargetStack.Quantity += AmountToMove;
		SourceStack.Quantity -= AmountToMove;

		if (SourceStack.Quantity <= 0)
		{
			SourceStack = FItemStack();
		}
	}
	else
	{
		// different items: swap the two stacks
		Swap(SourceStack, TargetStack);
	}

	BroadcastUpdate();

	if (TargetInventory != this)
	{
		TargetInventory->BroadcastUpdate();
	}

	return true;
}

bool UInventoryComponent::IsValidSlotIndex(int32 Index) const
{
	return Slots.IsValidIndex(Index);
}

void UInventoryComponent::BroadcastUpdate()
{
	OnInventoryUpdated.Broadcast();
}
