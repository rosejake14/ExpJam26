// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventorySlotWidget.h"
#include "InventoryComponent.h"
#include "ItemDragDropOperation.h"
#include "InputCoreTypes.h"

void UInventorySlotWidget::SetSlot(UInventoryComponent* InInventory, int32 InSlotIndex)
{
	if (Inventory)
	{
		Inventory->OnInventoryUpdated.RemoveDynamic(this, &UInventorySlotWidget::HandleInventoryUpdated);
	}

	Inventory = InInventory;
	SlotIndex = InSlotIndex;

	if (Inventory)
	{
		Inventory->OnInventoryUpdated.AddDynamic(this, &UInventorySlotWidget::HandleInventoryUpdated);
	}

	RefreshSlot();
}

void UInventorySlotWidget::NativeDestruct()
{
	if (Inventory)
	{
		Inventory->OnInventoryUpdated.RemoveDynamic(this, &UInventorySlotWidget::HandleInventoryUpdated);
	}

	Super::NativeDestruct();
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Inventory && !Inventory->GetSlot(SlotIndex).IsEmpty())
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!Inventory)
	{
		return;
	}

	const FItemStack Stack = Inventory->GetSlot(SlotIndex);

	if (Stack.IsEmpty())
	{
		return;
	}

	UItemDragDropOperation* DragOperation = NewObject<UItemDragDropOperation>(this);
	DragOperation->SourceInventory = Inventory;
	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->Stack = Stack;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->DefaultDragVisual = BP_CreateDragVisual(Stack);

	OutOperation = DragOperation;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UItemDragDropOperation* ItemOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (ItemOperation->SourceInventory && Inventory)
		{
			return ItemOperation->SourceInventory->MoveSlot(ItemOperation->SourceSlotIndex, Inventory, SlotIndex);
		}
	}

	return false;
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (Cast<UItemDragDropOperation>(InOperation))
	{
		BP_OnDragEnter();
	}
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (Cast<UItemDragDropOperation>(InOperation))
	{
		BP_OnDragLeave();
	}
}

void UInventorySlotWidget::HandleInventoryUpdated()
{
	RefreshSlot();
}

void UInventorySlotWidget::RefreshSlot()
{
	BP_UpdateSlot(Inventory ? Inventory->GetSlot(SlotIndex) : FItemStack());
}
