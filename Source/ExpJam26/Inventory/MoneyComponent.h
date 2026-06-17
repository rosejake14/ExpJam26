// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoneyComponent.generated.h"

/** Broadcast whenever the balance changes, so the HUD widget can refresh */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyChanged, int32, NewBalance);

/**
 *  Tracks a player's money balance. Add this component to the player character.
 *  Award money via AddMoney; deduct via SpendMoney.
 */
UCLASS(Blueprintable, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class EXPJAM26_API UMoneyComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMoneyComponent();

	/** Starting balance when the game begins */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Money", meta=(ClampMin=0))
	int32 StartingBalance = 0;

	/** Broadcast whenever the balance changes */
	UPROPERTY(BlueprintAssignable, Category="Money")
	FOnMoneyChanged OnMoneyChanged;

	/** Adds Amount to the balance and broadcasts OnMoneyChanged */
	UFUNCTION(BlueprintCallable, Category="Money")
	void AddMoney(int32 Amount);

	/**
	 * Deducts Amount from the balance if affordable.
	 * @return True if the deduction succeeded.
	 */
	UFUNCTION(BlueprintCallable, Category="Money")
	bool SpendMoney(int32 Amount);

	/** Returns the current balance */
	UFUNCTION(BlueprintPure, Category="Money")
	int32 GetBalance() const { return Balance; }

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, Category="Money")
	int32 Balance = 0;
};
