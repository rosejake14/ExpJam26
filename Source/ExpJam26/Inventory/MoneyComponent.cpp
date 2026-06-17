// Copyright Epic Games, Inc. All Rights Reserved.

#include "MoneyComponent.h"

UMoneyComponent::UMoneyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMoneyComponent::BeginPlay()
{
	Super::BeginPlay();
	Balance = FMath::Max(0, StartingBalance);
}

void UMoneyComponent::AddMoney(int32 Amount)
{
	if (Amount <= 0) { return; }
	Balance += Amount;
	OnMoneyChanged.Broadcast(Balance);
}

bool UMoneyComponent::SpendMoney(int32 Amount)
{
	if (Amount <= 0 || Balance < Amount) { return false; }
	Balance -= Amount;
	OnMoneyChanged.Broadcast(Balance);
	return true;
}
