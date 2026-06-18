// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExpJam26Character.h"
#include "HorrorCharacter.generated.h"

class USpotLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);

/**
 *  Simple first person horror character
 *  Provides stamina-based sprinting
 */
UCLASS(abstract)
class EXPJAM26_API AHorrorCharacter : public AExpJam26Character
{
	GENERATED_BODY()

	/** Player light source */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;
	
protected:

	/** If true, we're recovering stamina */
	bool bRecovering = false;

	/** Time interval for sprinting stamina ticks */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float SprintFixedTickTime = 0.03333f;

	/** Sprint stamina amount. Maxes at SprintTime */
	float SprintMeter = 0.0f;

	/** How long we can sprint for, in seconds */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float SprintTime = 3.0f;

	/** Walk speed while recovering stamina */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float RecoveringWalkSpeed = 150.0f;

	/** Time it takes for the sprint meter to recover */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RecoveryTime = 0.0f;

	/** Sprint tick timer */
	FTimerHandle SprintTimer;

public:

	/** Delegate called when the sprint meter should be updated */
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** Delegate called when we start and stop sprinting */
	FSprintStateChangedDelegate OnSprintStateChanged;

protected:

	/** Constructor */
	AHorrorCharacter();

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Starts sprinting behavior (stamina-based override) */
	virtual void DoStartSprint() override;

	/** Stops sprinting behavior (stamina-based override) */
	virtual void DoEndSprint() override;

	/** Called while sprinting at a fixed time interval */
	void SprintFixedTick();
};
