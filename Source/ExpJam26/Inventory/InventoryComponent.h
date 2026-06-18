// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDefinition.h"
#include "InventoryComponent.generated.h"

/** Broadcast whenever the contents of the inventory change, so UI can refresh */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**
 *  A fixed-size grid of item stacks. Add this to any Actor to give it an inventory
 *  that can receive items from AItemPickup, be modified by UCraftingComponent, and
 *  be displayed/rearranged via UInventoryWidget.
 */
UCLASS(Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class EXPJAM26_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInventoryComponent();

	/** Number of slots in this inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta = (ClampMin = 1))
	int32 NumSlots = 20;

	/** Items granted to this inventory when it begins play */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<FItemStack> StartingItems;

	/** Broadcast whenever a slot's contents change */
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:

	/** The item stacks held by this inventory. Size is always NumSlots. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<FItemStack> Slots;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/**
	 * Adds up to Quantity of Item to this inventory, merging into existing stacks
	 * first and then filling empty slots.
	 * @return The quantity that could not be added (0 if everything was added).
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(UItemDefinition* Item, int32 Quantity);

	/**
	 * Removes Quantity of Item from this inventory across however many slots
	 * are needed. Fails (and removes nothing) if the inventory doesn't contain
	 * at least Quantity of Item.
	 * @return True if the items were removed.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(UItemDefinition* Item, int32 Quantity);

	/**
	 * Clears the entire stack in the given slot, regardless of item type.
	 * @return True if the slot was non-empty and was cleared.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveSlot(int32 SlotIndex);

	/** Returns the total quantity of Item held across all slots */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetItemCount(UItemDefinition* Item) const;

	/** Returns the contents of the given slot, or an empty stack if out of range */
	UFUNCTION(BlueprintPure, Category="Inventory")
	FItemStack GetSlot(int32 SlotIndex) const;

	/** Returns the number of slots in this inventory */
	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetNumSlots() const;

	/**
	 * Moves the stack in SourceSlot of this inventory into TargetSlot of TargetInventory
	 * (which may be this inventory). If the target slot is empty, the stack is moved.
	 * If it holds the same item, the stacks are merged up to MaxStackSize. Otherwise
	 * the two stacks are swapped.
	 * @return True if the move was performed.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool MoveSlot(int32 SourceSlot, UInventoryComponent* TargetInventory, int32 TargetSlot);

private:

	/** Returns true if Index is a valid slot index */
	bool IsValidSlotIndex(int32 Index) const;

	/** Broadcasts OnInventoryUpdated */
	void BroadcastUpdate();
};
