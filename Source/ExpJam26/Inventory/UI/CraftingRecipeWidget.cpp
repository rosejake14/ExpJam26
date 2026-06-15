// Copyright Epic Games, Inc. All Rights Reserved.


#include "CraftingRecipeWidget.h"
#include "CraftingComponent.h"
#include "CraftingRecipe.h"

void UCraftingRecipeWidget::SetRecipe(UCraftingComponent* InCrafting, UCraftingRecipe* InRecipe)
{
	Crafting = InCrafting;
	Recipe = InRecipe;

	RefreshRecipe();
}

void UCraftingRecipeWidget::TryCraft()
{
	if (Crafting && Recipe)
	{
		Crafting->CraftItem(Recipe);
	}

	RefreshRecipe();
}

void UCraftingRecipeWidget::RefreshRecipe()
{
	const bool bCanCraft = Crafting && Recipe && Crafting->CanCraft(Recipe);

	BP_UpdateRecipe(bCanCraft);
}
