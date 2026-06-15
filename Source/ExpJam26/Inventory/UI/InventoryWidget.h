// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UPanelWidget;

/**
 *  Displays the slots of an UInventoryComponent as a grid of UInventorySlotWidget.
 */
UCLASS(abstract)
class EXPJAM26_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Assigns the inventory to display and (re)builds its slot widgets */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetInventory(UInventoryComponent* InInventory);

	/** The inventory currently displayed by this widget */
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

protected:

	/** Widget class spawned for each slot in Inventory */
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	/** Panel that slot widgets are added to. Add a panel (e.g. a WrapBox) with this name in the widget designer. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UPanelWidget* SlotContainer;

	/** Removes any existing slot widgets and spawns one SlotWidgetClass per slot of Inventory */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void BuildSlots();
};
