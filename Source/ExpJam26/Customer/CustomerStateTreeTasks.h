// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "CustomerStateTreeTasks.generated.h"

class ACustomerNPC;
class ACustomerAIController;

////////////////////////////////////////////////////////////////////
// CONDITION: Is Being Interacted
////////////////////////////////////////////////////////////////////

USTRUCT()
struct FStateTreeCustomerInteractConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerNPC> Character;
};

/** Passes when the customer NPC is currently in dialogue with the player */
USTRUCT(DisplayName="Customer Is Being Interacted", Category="Customer")
struct FStateTreeCustomerInteractCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCustomerInteractConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// CONDITION: Wants To Go To Shop
////////////////////////////////////////////////////////////////////

USTRUCT()
struct FStateTreeCustomerWantsShopConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerNPC> Character;
};

/** Passes when the customer NPC has decided to visit the player's shop */
USTRUCT(DisplayName="Customer Wants To Go To Shop", Category="Customer")
struct FStateTreeCustomerWantsShopCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCustomerWantsShopConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TASK: Interact
////////////////////////////////////////////////////////////////////

USTRUCT()
struct FStateTreeCustomerInteractTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerAIController> Controller;

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerNPC> Character;

	/** Set to true by the OnCustomerInteractionEnded lambda to signal task completion */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bInteractionEnded = false;
};

/** Blocks NPC movement while the player is in dialogue. Completes when dialogue ends. */
USTRUCT(meta=(DisplayName="Customer Interact", Category="Customer"))
struct FStateTreeCustomerInteractTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCustomerInteractTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TASK: Roam
////////////////////////////////////////////////////////////////////

USTRUCT()
struct FStateTreeCustomerRoamTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerAIController> Controller;

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerNPC> Character;

	/** Minimum wait time at destination before picking the next roam point */
	UPROPERTY(EditAnywhere, Category="Parameter")
	float MinWaitTime = 3.0f;

	/** Maximum wait time at destination before picking the next roam point */
	UPROPERTY(EditAnywhere, Category="Parameter")
	float MaxWaitTime = 8.0f;

	/** Set to true by the move-completed lambda when the NPC arrives at its roam point */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bIsWaiting = false;

	/** Remaining seconds before the NPC picks a new roam destination */
	UPROPERTY(EditAnywhere, Category="Output")
	float WaitTimeRemaining = 0.0f;
};

/** Picks a random reachable point within the NPC's roam area, walks there, and waits. */
USTRUCT(meta=(DisplayName="Customer Roam", Category="Customer"))
struct FStateTreeCustomerRoamTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCustomerRoamTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TASK: Go To Shop
////////////////////////////////////////////////////////////////////

USTRUCT()
struct FStateTreeGoToShopTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerAIController> Controller;

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<ACustomerNPC> Character;

	/** True once the NPC has successfully joined the shop queue */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bHasJoinedQueue = false;

	/** True once the NPC has reached the front of the queue and is waiting */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bIsWaitingAtFront = false;

	/** Set by the move-completed lambda each time the NPC finishes a movement */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bMoveCompleted = false;

	/** True after LeaveQueue has been called, prevents double-removal in ExitState */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bAlreadyLeftQueue = false;

	/** Remaining seconds before the NPC leaves the front of the queue */
	UPROPERTY(EditAnywhere, Category="Output")
	float PurchaseTimeRemaining = 0.0f;

	/** True when this shop visit was triggered by an accepted order request; NPC waits until the order is fulfilled */
	UPROPERTY(EditAnywhere, Category="Output")
	bool bWasOrderVisit = false;
};

/** Joins the shop queue, walks to the assigned slot, waits when at the front, then leaves. */
USTRUCT(meta=(DisplayName="Go To Shop", Category="Customer"))
struct FStateTreeGoToShopTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeGoToShopTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};
