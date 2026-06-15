// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftingWidget.generated.h"

class UCraftingComponent;
class UCraftingRecipeWidget;
class UPanelWidget;

/**
 *  Displays the recipes of a UCraftingComponent as a list of UCraftingRecipeWidget,
 *  and keeps them up to date as the linked inventory changes.
 */
UCLASS(abstract)
class EXPJAM26_API UCraftingWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Assigns the crafting component to display and (re)builds its recipe list */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void SetCraftingComponent(UCraftingComponent* InCrafting);

	/** The crafting component currently displayed by this widget */
	UPROPERTY(BlueprintReadOnly, Category="Crafting")
	TObjectPtr<UCraftingComponent> Crafting;

protected:

	/** Widget class spawned for each recipe in Crafting's AvailableRecipes */
	UPROPERTY(EditAnywhere, Category="Crafting")
	TSubclassOf<UCraftingRecipeWidget> RecipeWidgetClass;

	/** Panel that recipe widgets are added to. Add a panel (e.g. a VerticalBox) with this name in the widget designer. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UPanelWidget* RecipeContainer;

	/** Unbinds from the linked inventory's update delegate */
	virtual void NativeDestruct() override;

	/** Removes any existing recipe widgets and spawns one RecipeWidgetClass per available recipe */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void BuildRecipeList();

	/** Bound to the linked inventory's update delegate; refreshes all recipe widgets */
	UFUNCTION()
	void HandleInventoryUpdated();
};
