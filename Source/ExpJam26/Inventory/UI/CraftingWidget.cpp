// Copyright Epic Games, Inc. All Rights Reserved.


#include "CraftingWidget.h"
#include "CraftingComponent.h"
#include "CraftingRecipe.h"
#include "CraftingRecipeWidget.h"
#include "InventoryComponent.h"
#include "Components/PanelWidget.h"

void UCraftingWidget::SetCraftingComponent(UCraftingComponent* InCrafting)
{
	if (Crafting)
	{
		if (UInventoryComponent* OldInventory = Crafting->GetLinkedInventory())
		{
			OldInventory->OnInventoryUpdated.RemoveDynamic(this, &UCraftingWidget::HandleInventoryUpdated);
		}
	}

	Crafting = InCrafting;

	if (Crafting)
	{
		if (UInventoryComponent* NewInventory = Crafting->GetLinkedInventory())
		{
			NewInventory->OnInventoryUpdated.AddDynamic(this, &UCraftingWidget::HandleInventoryUpdated);
		}
	}

	BuildRecipeList();
}

void UCraftingWidget::NativeDestruct()
{
	if (Crafting)
	{
		if (UInventoryComponent* LinkedInventory = Crafting->GetLinkedInventory())
		{
			LinkedInventory->OnInventoryUpdated.RemoveDynamic(this, &UCraftingWidget::HandleInventoryUpdated);
		}
	}

	Super::NativeDestruct();
}

void UCraftingWidget::BuildRecipeList()
{
	if (!RecipeContainer || !RecipeWidgetClass || !Crafting)
	{
		return;
	}

	RecipeContainer->ClearChildren();

	for (UCraftingRecipe* Recipe : Crafting->AvailableRecipes)
	{
		if (!Recipe)
		{
			continue;
		}

		if (UCraftingRecipeWidget* RecipeWidget = CreateWidget<UCraftingRecipeWidget>(this, RecipeWidgetClass))
		{
			RecipeWidget->SetRecipe(Crafting, Recipe);
			RecipeContainer->AddChild(RecipeWidget);
		}
	}
}

void UCraftingWidget::HandleInventoryUpdated()
{
	if (!RecipeContainer)
	{
		return;
	}

	for (int32 Index = 0; Index < RecipeContainer->GetChildrenCount(); ++Index)
	{
		if (UCraftingRecipeWidget* RecipeWidget = Cast<UCraftingRecipeWidget>(RecipeContainer->GetChildAt(Index)))
		{
			RecipeWidget->RefreshRecipe();
		}
	}
}
