// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventorySlotWidget.h"
#include "InventoryComponent.h"
#include "ItemDragDropOperation.h"
#include "InputCoreTypes.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "ExpJam26.h"

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SlotIcon     = Cast<UImage>(GetWidgetFromName(TEXT("Icon")));
	SlotQuantity = Cast<UTextBlock>(GetWidgetFromName(TEXT("Quantity")));

	// If name lookup failed, find by type and log real names so we can fix them
	if (!SlotIcon || !SlotQuantity)
	{
		WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			UE_LOG(LogExpJam26, Log, TEXT("  SlotWidget child: '%s' (%s)"),
				*Widget->GetName(), *Widget->GetClass()->GetName());

			if (!SlotIcon)     { SlotIcon     = Cast<UImage>(Widget); }
			if (!SlotQuantity) { SlotQuantity = Cast<UTextBlock>(Widget); }
		});
	}

	UE_LOG(LogExpJam26, Log, TEXT("InventorySlotWidget NativeConstruct — Icon: %s, Quantity: %s"),
		SlotIcon     ? TEXT("OK") : TEXT("NULL"),
		SlotQuantity ? TEXT("OK") : TEXT("NULL"));
}

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

void UInventorySlotWidget::BP_UpdateSlot_Implementation(const FItemStack& Stack)
{
	const bool bHasItem = Stack.Item != nullptr && Stack.Quantity > 0;

	UE_LOG(LogExpJam26, Log, TEXT("BP_UpdateSlot slot %d — Item: %s, Icon widget: %s"),
		SlotIndex,
		bHasItem ? *Stack.Item->DisplayName.ToString() : TEXT("empty"),
		SlotIcon ? TEXT("OK") : TEXT("NULL"));

	if (SlotIcon)
	{
		if (bHasItem)
		{
			SlotIcon->SetBrushFromTexture(Stack.Item->Icon.Get());
			SlotIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			SlotIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (SlotQuantity)
	{
		if (bHasItem && Stack.Quantity > 1)
		{
			SlotQuantity->SetText(FText::AsNumber(Stack.Quantity));
			SlotQuantity->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			SlotQuantity->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
