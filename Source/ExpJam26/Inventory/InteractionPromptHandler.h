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

	/**
	 * Registers a prompt from Source with the given text.
	 * The player tracks all active sources; whichever registered most recently is shown.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void ShowInteractionPrompt(UObject* Source, const FText& PromptText);

	/**
	 * Removes Source's prompt registration.
	 * If other sources are still active, their prompt is restored; otherwise the widget hides.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void HideInteractionPrompt(UObject* Source);
};
