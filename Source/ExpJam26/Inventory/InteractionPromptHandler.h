// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionPromptHandler.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionPromptHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Implemented by player characters/controllers that can display an
 *  on-screen prompt (e.g. "Press E to pick up Wood") for nearby interactables.
 */
class EXPJAM26_API IInteractionPromptHandler
{
	GENERATED_BODY()

public:

	/** Shows an interaction prompt with the given text, e.g. "Press E to pick up Wood" */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void ShowInteractionPrompt(const FText& PromptText);

	/** Hides the interaction prompt */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void HideInteractionPrompt();
};
