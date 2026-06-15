// Copyright Epic Games, Inc. All Rights Reserved.


#include "CraftingComponent.h"
#include "CraftingRecipe.h"
#include "InventoryComponent.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		LinkedInventory = Owner->FindComponentByClass<UInventoryComponent>();
	}
}

bool UCraftingComponent::CanCraft(UCraftingRecipe* Recipe) const
{
	if (!Recipe || !LinkedInventory)
	{
		return false;
	}

	for (const FItemStack& Ingredient : Recipe->Ingredients)
	{
		if (Ingredient.IsEmpty())
		{
			continue;
		}

		if (LinkedInventory->GetItemCount(Ingredient.Item) < Ingredient.Quantity)
		{
			return false;
		}
	}

	return true;
}

bool UCraftingComponent::CraftItem(UCraftingRecipe* Recipe)
{
	if (!CanCraft(Recipe))
	{
		return false;
	}

	for (const FItemStack& Ingredient : Recipe->Ingredients)
	{
		if (!Ingredient.IsEmpty())
		{
			LinkedInventory->RemoveItem(Ingredient.Item, Ingredient.Quantity);
		}
	}

	if (!Recipe->Result.IsEmpty())
	{
		LinkedInventory->AddItem(Recipe->Result.Item, Recipe->Result.Quantity);
	}

	OnRecipeCrafted.Broadcast(Recipe);

	return true;
}
