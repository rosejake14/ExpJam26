// Copyright Epic Games, Inc. All Rights Reserved.


#include "CustomerAIController.h"
#include "CustomerNPC.h"
#include "Components/StateTreeAIComponent.h"

ACustomerAIController::ACustomerAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
}

void ACustomerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedNPC = Cast<ACustomerNPC>(InPawn);

	if (PossessedNPC)
	{
		PossessedNPC->OnInteractionStarted.AddDynamic(this, &ACustomerAIController::HandleInteractionStarted);
		PossessedNPC->OnInteractionEnded.AddDynamic(this, &ACustomerAIController::HandleInteractionEnded);
		PossessedNPC->OnWantsToGoToShop.AddDynamic(this, &ACustomerAIController::HandleWantsToGoToShop);
	}

	ReceiveMoveCompleted.AddDynamic(this, &ACustomerAIController::HandleMoveCompleted);
}

void ACustomerAIController::OnUnPossess()
{
	if (PossessedNPC)
	{
		PossessedNPC->OnInteractionStarted.RemoveDynamic(this, &ACustomerAIController::HandleInteractionStarted);
		PossessedNPC->OnInteractionEnded.RemoveDynamic(this, &ACustomerAIController::HandleInteractionEnded);
		PossessedNPC->OnWantsToGoToShop.RemoveDynamic(this, &ACustomerAIController::HandleWantsToGoToShop);
		PossessedNPC = nullptr;
	}

	ReceiveMoveCompleted.RemoveDynamic(this, &ACustomerAIController::HandleMoveCompleted);

	Super::OnUnPossess();
}

void ACustomerAIController::HandleInteractionStarted()
{
	OnCustomerInteractionStarted.ExecuteIfBound();
}

void ACustomerAIController::HandleInteractionEnded()
{
	OnCustomerInteractionEnded.ExecuteIfBound();
}

void ACustomerAIController::HandleWantsToGoToShop()
{
	OnCustomerWantsToGoToShop.ExecuteIfBound();
}

void ACustomerAIController::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	OnCustomerMoveCompleted.ExecuteIfBound();
}
