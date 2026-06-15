// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

class UCraftingRecipe;
class UInventoryComponent;

/** Broadcast whenever a recipe is successfully crafted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeCrafted, UCraftingRecipe*, Recipe);

/**
 *  Allows its owner to craft items by consuming ingredients from a sibling
 *  UInventoryComponent and adding the recipe's result to it.
 */
UCLASS(Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class EXPJAM26_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCraftingComponent();

	/** Recipes this component is able to craft */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	TArray<TObjectPtr<UCraftingRecipe>> AvailableRecipes;

	/** Broadcast whenever a recipe is successfully crafted */
	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnRecipeCrafted OnRecipeCrafted;

protected:

	/** The inventory recipes are crafted from and into. Found on the owner at BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	TObjectPtr<UInventoryComponent> LinkedInventory;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Returns true if LinkedInventory holds every ingredient Recipe requires */
	UFUNCTION(BlueprintPure, Category="Crafting")
	bool CanCraft(UCraftingRecipe* Recipe) const;

	/**
	 * If CanCraft(Recipe), removes its ingredients from LinkedInventory and
	 * adds its result.
	 * @return True if the recipe was crafted.
	 */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool CraftItem(UCraftingRecipe* Recipe);

	/** Returns the inventory recipes are crafted from and into */
	UFUNCTION(BlueprintPure, Category="Crafting")
	UInventoryComponent* GetLinkedInventory() const { return LinkedInventory; }
};
