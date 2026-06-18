// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UStaticMesh;
class UTexture2D;

/**
 *  Defines a type of item that can be held in an inventory, picked up, or crafted.
 *  Designers create instances of this (or Blueprint subclasses) as Data Assets,
 *  one per item type.
 */
UCLASS(Blueprintable, BlueprintType)
class EXPJAM26_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Name shown to the player in inventory and crafting UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	/** Description shown to the player in inventory and crafting UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta = (MultiLine = "true"))
	FText Description;

	/** Icon shown in inventory and crafting UI slots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	TObjectPtr<UTexture2D> Icon;

	/** Mesh used to represent this item when it's dropped in the world as a pickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UStaticMesh> PickupMesh;

	/** Maximum number of this item that can be held in a single inventory slot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 1;
};

/**
 *  A stack of a particular item and how many of it there are.
 *  Used to represent inventory slots, pickups, and crafting ingredients/results.
 */
USTRUCT(BlueprintType)
struct FItemStack
{
	GENERATED_BODY()

	/** The type of item in this stack. If null, the stack is considered empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	TObjectPtr<UItemDefinition> Item = nullptr;

	/** How many of Item are in this stack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	int32 Quantity = 0;

	FItemStack() = default;

	FItemStack(UItemDefinition* InItem, int32 InQuantity)
		: Item(InItem)
		, Quantity(InQuantity)
	{}

	/** Returns true if this stack has no item or no quantity */
	bool IsEmpty() const
	{
		return Item == nullptr || Quantity <= 0;
	}
};
