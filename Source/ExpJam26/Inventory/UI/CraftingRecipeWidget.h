// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftingRecipeWidget.generated.h"

class UCraftingComponent;
class UCraftingRecipe;

/**
 *  Displays a single UCraftingRecipe and allows the player to craft it via
 *  a Blueprint-implemented "Craft" button calling TryCraft.
 */
UCLASS(abstract)
class EXPJAM26_API UCraftingRecipeWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Assigns the recipe and crafting component this widget represents, and refreshes its display */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void SetRecipe(UCraftingComponent* InCrafting, UCraftingRecipe* InRecipe);

	/** Attempts to craft Recipe via Crafting, then refreshes this widget's display */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void TryCraft();

	/** Re-evaluates whether Recipe can currently be crafted and updates this widget's visuals */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void RefreshRecipe();

protected:

	/** The recipe this widget represents */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	TObjectPtr<UCraftingRecipe> Recipe;

	/** The crafting component Recipe is crafted through */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	TObjectPtr<UCraftingComponent> Crafting;

	/** Passes control to Blueprint to update this widget's visuals */
	UFUNCTION(BlueprintImplementableEvent, Category="Crafting", meta = (DisplayName = "Update Recipe"))
	void BP_UpdateRecipe(bool bCanCraft);
};
