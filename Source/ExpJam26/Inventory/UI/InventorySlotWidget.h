// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDefinition.h"
#include "InventorySlotWidget.generated.h"

class UInventoryComponent;
class UDragDropOperation;
class UImage;
class UTextBlock;

/**
 *  Represents a single slot of an inventory in UI. Supports dragging its
 *  stack out to another slot, and receiving a stack dragged in from another
 *  slot (in the same or a different inventory).
 */
UCLASS(abstract)
class EXPJAM26_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Assigns the inventory and slot index this widget represents, and refreshes its display */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetSlot(UInventoryComponent* InInventory, int32 InSlotIndex);

	/** The inventory this slot belongs to */
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	/** The index of this slot within Inventory */
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	int32 SlotIndex = -1;

protected:

	/** Resolves Icon and Quantity widget references by name */
	virtual void NativeConstruct() override;

	/** Unbinds from Inventory's update delegate */
	virtual void NativeDestruct() override;

	/** Begins a drag if the left mouse button is pressed on a non-empty slot */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Builds the drag and drop payload for this slot's stack */
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	/** Moves the dragged stack into this slot */
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** Notifies Blueprint that a stack is being dragged over this slot */
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** Notifies Blueprint that a dragged stack has left this slot */
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UImage* SlotIcon = nullptr;
	UTextBlock* SlotQuantity = nullptr;

	/** Bound to Inventory's update delegate, refreshes this slot's display */
	UFUNCTION()
	void HandleInventoryUpdated();

	/** Refreshes this slot's display from the current contents of Inventory/SlotIndex */
	void RefreshSlot();

	/** Updates this slot's visuals from the given stack. C++ handles it; Blueprint may override. */
	UFUNCTION(BlueprintNativeEvent, Category="Inventory", meta = (DisplayName = "Update Slot"))
	void BP_UpdateSlot(const FItemStack& Stack);

	/** Lets Blueprint build the floating widget shown while dragging this slot's stack */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory", meta = (DisplayName = "Create Drag Visual"))
	UWidget* BP_CreateDragVisual(const FItemStack& Stack);

	/** Called when a dragged stack enters this slot's bounds, for hover highlight */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory", meta = (DisplayName = "On Drag Enter"))
	void BP_OnDragEnter();

	/** Called when a dragged stack leaves this slot's bounds, for hover highlight */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory", meta = (DisplayName = "On Drag Leave"))
	void BP_OnDragLeave();
};
