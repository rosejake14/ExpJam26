// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "Components/PanelWidget.h"

void UInventoryWidget::SetInventory(UInventoryComponent* InInventory)
{
	Inventory = InInventory;

	BuildSlots();
}

void UInventoryWidget::BuildSlots()
{
	if (!SlotContainer || !SlotWidgetClass || !Inventory)
	{
		return;
	}

	SlotContainer->ClearChildren();

	for (int32 SlotIndex = 0; SlotIndex < Inventory->GetNumSlots(); ++SlotIndex)
	{
		if (UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass))
		{
			SlotContainer->AddChild(SlotWidget);   // NativeConstruct fires here, resolving widget refs
			SlotWidget->SetSlot(Inventory, SlotIndex);
		}
	}
}
