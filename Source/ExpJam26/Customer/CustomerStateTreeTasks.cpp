// Copyright Epic Games, Inc. All Rights Reserved.


#include "CustomerStateTreeTasks.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeAsyncExecutionContext.h"
#include "CustomerNPC.h"
#include "CustomerAIController.h"
#include "ShopQueueComponent.h"
#include "NavigationSystem.h"

////////////////////////////////////////////////////////////////////
// CONDITIONS
////////////////////////////////////////////////////////////////////

bool FStateTreeCustomerInteractCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return IsValid(InstanceData.Character) && InstanceData.Character->bIsBeingInteracted;
}

#if WITH_EDITOR
FText FStateTreeCustomerInteractCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Customer Is Being Interacted</b>");
}
#endif

////////////////////////////////////////////////////////////////////

bool FStateTreeCustomerWantsShopCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return IsValid(InstanceData.Character) && InstanceData.Character->bWantsToGoToShop;
}

#if WITH_EDITOR
FText FStateTreeCustomerWantsShopCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Customer Wants To Go To Shop</b>");
}
#endif

////////////////////////////////////////////////////////////////////
// TASK: Interact
////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeCustomerInteractTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.bInteractionEnded = false;

		// halt movement while the player is talking to us
		InstanceData.Controller->StopMovement();

		// when dialogue ends, set the flag so Tick can return Succeeded
		InstanceData.Controller->OnCustomerInteractionEnded.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()]()
			{
				if (FInstanceDataType* LambdaData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>())
				{
					LambdaData->bInteractionEnded = true;
				}
			}
		);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeCustomerInteractTask::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.bInteractionEnded ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FStateTreeCustomerInteractTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.Controller->OnCustomerInteractionEnded.Unbind();
	}
}

#if WITH_EDITOR
FText FStateTreeCustomerInteractTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Customer Interact</b>");
}
#endif

////////////////////////////////////////////////////////////////////
// TASK: Roam
////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeCustomerRoamTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.bIsWaiting = false;
		InstanceData.WaitTimeRemaining = 0.0f;

		// pick a random reachable point within the NPC's roam zone
		const FVector RoamCenter = InstanceData.Character->GetEffectiveRoamCenter();
		const float RoamRadius = InstanceData.Character->RoamRadius;
		const FVector CurrentLocation = InstanceData.Character->GetActorLocation();

		FVector TargetLocation = CurrentLocation;

		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InstanceData.Character->GetWorld());
		if (NavSys)
		{
			FNavLocation NavLocation;
			if (NavSys->GetRandomReachablePointInRadius(RoamCenter, RoamRadius, NavLocation))
			{
				TargetLocation = NavLocation.Location;
			}
			else
			{
				// zone query failed — wander from current position at a smaller radius
				NavSys->GetRandomReachablePointInRadius(CurrentLocation, RoamRadius * 0.5f, NavLocation);
				TargetLocation = NavLocation.Location;
			}
		}

		InstanceData.Controller->MoveToLocation(TargetLocation);

		// when the NPC arrives, start the idle wait timer
		InstanceData.Controller->OnCustomerMoveCompleted.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()]()
			{
				if (FInstanceDataType* LambdaData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>())
				{
					LambdaData->bIsWaiting = true;
					LambdaData->WaitTimeRemaining = FMath::RandRange(LambdaData->MinWaitTime, LambdaData->MaxWaitTime);
				}
			}
		);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeCustomerRoamTask::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bIsWaiting)
	{
		InstanceData.WaitTimeRemaining -= DeltaTime;

		if (InstanceData.WaitTimeRemaining <= 0.0f)
		{
			// one roam cycle complete — may trigger GoToShop on the NPC
			InstanceData.Character->OnRoamCycleCompleted();
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeCustomerRoamTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.Controller->OnCustomerMoveCompleted.Unbind();
	}
}

#if WITH_EDITOR
FText FStateTreeCustomerRoamTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Customer Roam</b>");
}
#endif

////////////////////////////////////////////////////////////////////
// TASK: Go To Shop
////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeGoToShopTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.bHasJoinedQueue = false;
		InstanceData.bIsWaitingAtFront = false;
		InstanceData.bMoveCompleted = false;
		InstanceData.bAlreadyLeftQueue = false;
		InstanceData.PurchaseTimeRemaining = 0.0f;

		ACustomerNPC* Character = InstanceData.Character;

		if (!IsValid(Character->ShopActor))
		{
			return EStateTreeRunStatus::Failed;
		}

		UShopQueueComponent* Queue = Character->ShopActor->FindComponentByClass<UShopQueueComponent>();
		if (!Queue)
		{
			return EStateTreeRunStatus::Failed;
		}

		const FVector SlotPosition = Queue->JoinQueue(Character);
		InstanceData.bHasJoinedQueue = true;
		InstanceData.bWasOrderVisit = Character->bHasActiveRequest;

		InstanceData.Controller->MoveToLocation(SlotPosition);

		// each time movement completes (initial arrival or queue advance), set the flag for Tick
		InstanceData.Controller->OnCustomerMoveCompleted.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()]()
			{
				if (FInstanceDataType* LambdaData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>())
				{
					LambdaData->bMoveCompleted = true;
				}
			}
		);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeGoToShopTask::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ACustomerNPC* Character = InstanceData.Character;

	// waiting at the front of the queue
	if (InstanceData.bIsWaitingAtFront)
	{
		if (InstanceData.bWasOrderVisit)
		{
			// hold position until the player delivers the requested item
			if (!Character->bHasActiveRequest)
			{
				if (IsValid(Character->ShopActor))
				{
					if (UShopQueueComponent* Queue = Character->ShopActor->FindComponentByClass<UShopQueueComponent>())
					{
						Queue->LeaveQueue(Character);
						InstanceData.bAlreadyLeftQueue = true;
					}
				}

				Character->bWantsToGoToShop = false;
				return EStateTreeRunStatus::Succeeded;
			}

			return EStateTreeRunStatus::Running;
		}

		// normal (non-order) shop visit: count down and leave
		InstanceData.PurchaseTimeRemaining -= DeltaTime;

		if (InstanceData.PurchaseTimeRemaining <= 0.0f)
		{
			if (IsValid(Character->ShopActor))
			{
				if (UShopQueueComponent* Queue = Character->ShopActor->FindComponentByClass<UShopQueueComponent>())
				{
					Queue->LeaveQueue(Character);
					InstanceData.bAlreadyLeftQueue = true;
				}
			}

			Character->bWantsToGoToShop = false;
			return EStateTreeRunStatus::Succeeded;
		}

		return EStateTreeRunStatus::Running;
	}

	// movement just completed — check whether we've reached the front
	if (InstanceData.bMoveCompleted)
	{
		InstanceData.bMoveCompleted = false;

		if (IsValid(Character->ShopActor))
		{
			if (UShopQueueComponent* Queue = Character->ShopActor->FindComponentByClass<UShopQueueComponent>())
			{
				if (Queue->IsAtFrontOfQueue(Character))
				{
					InstanceData.bIsWaitingAtFront = true;
					InstanceData.PurchaseTimeRemaining = FMath::RandRange(Character->MinShopWaitTime, Character->MaxShopWaitTime);
				}
				// else: still mid-queue; AdvanceToQueuePosition will trigger MoveCompleted again
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeGoToShopTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.Controller->OnCustomerMoveCompleted.Unbind();

		// safety: if interrupted while still in the queue, remove ourselves so others can advance
		if (InstanceData.bHasJoinedQueue && !InstanceData.bAlreadyLeftQueue)
		{
			ACustomerNPC* Character = InstanceData.Character;
			if (IsValid(Character->ShopActor))
			{
				if (UShopQueueComponent* Queue = Character->ShopActor->FindComponentByClass<UShopQueueComponent>())
				{
					Queue->LeaveQueue(Character);
				}
			}
			// leave bWantsToGoToShop = true so the NPC re-joins after the interruption ends
		}
	}
}

#if WITH_EDITOR
FText FStateTreeGoToShopTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Go To Shop</b>");
}
#endif
