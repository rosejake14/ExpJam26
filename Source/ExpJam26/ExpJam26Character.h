// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ExpJam26Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class AExpJam26Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SprintAction;

	/** Toggle destroy-mode Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DestroyModeAction;

	/** Destroy item in slot 1 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DestroySlot1Action;

	/** Destroy item in slot 2 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DestroySlot2Action;

	/** Destroy item in slot 3 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DestroySlot3Action;

	/** Whether destroy mode is currently active */
	bool bDestroyModeActive = false;

	/** Walk speed when not sprinting */
	UPROPERTY(EditAnywhere, Category="Movement")
	float WalkSpeed = 350.0f;

	/** Walk speed when sprinting */
	UPROPERTY(EditAnywhere, Category="Movement")
	float SprintSpeed = 650.0f;

	/** Whether currently sprinting */
	bool bSprinting = false;

public:
	AExpJam26Character();

protected:

	virtual void BeginPlay() override;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Handles sprint start from input or UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartSprint();

	/** Handles sprint end from input or UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoEndSprint();

	/** Toggles destroy mode on/off */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleDestroyMode();

	/** Called when destroy mode changes; override in Blueprint to update UI */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void BP_OnDestroyModeChanged(bool bActive);

private:

	/** Removes all items in SlotIndex if destroy mode is active, then exits destroy mode */
	void DoDestroySlot(int32 SlotIndex);

	void DoDestroySlot1() { DoDestroySlot(0); }
	void DoDestroySlot2() { DoDestroySlot(1); }
	void DoDestroySlot3() { DoDestroySlot(2); }

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

