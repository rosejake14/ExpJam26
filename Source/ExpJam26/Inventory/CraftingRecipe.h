// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.h"
#include "CraftingRecipe.generated.h"

/**
 *  Defines a crafting recipe: a set of item stacks consumed from an inventory
 *  in exchange for a resulting item stack. Designers create instances of this
 *  (or Blueprint subclasses) as Data Assets, one per recipe.
 */
UCLASS(Blueprintable, BlueprintType)
class EXPJAM26_API UCraftingRecipe : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Name shown to the player in crafting UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recipe")
	FText RecipeName;

	/** Items and quantities required to craft this recipe */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recipe")
	TArray<FItemStack> Ingredients;

	/** Item and quantity produced by this recipe */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recipe")
	FItemStack Result;
};
