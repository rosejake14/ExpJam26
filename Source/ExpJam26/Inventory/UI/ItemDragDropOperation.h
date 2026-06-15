// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDefinition.h"
#include "ItemDragDropOperation.generated.h"

class UInventoryComponent;

/**
 *  Drag and drop payload used when dragging an item stack out of an
 *  inventory slot widget.
 */
UCLASS()
class EXPJAM26_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:

	/** The inventory the dragged stack came from */
	UPROPERTY(BlueprintReadOnly, Category="Drag and Drop")
	TObjectPtr<UInventoryComponent> SourceInventory;

	/** The slot index in SourceInventory the dragged stack came from */
	UPROPERTY(BlueprintReadOnly, Category="Drag and Drop")
	int32 SourceSlotIndex = -1;

	/** A copy of the item stack being dragged, for building the drag visual */
	UPROPERTY(BlueprintReadOnly, Category="Drag and Drop")
	FItemStack Stack;
};
