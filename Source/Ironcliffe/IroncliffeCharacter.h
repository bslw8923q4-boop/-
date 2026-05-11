// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Systems/IroncliffeTypes.h"
#include "IroncliffeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UIroncliffeCombatComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS()
class AIroncliffeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Ironcliffe combat source of truth for stamina, morale, orders, and stance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UIroncliffeCombatComponent* CombatComponent;

	/** Prototype black cloak marker until Sirius' final skeletal mesh exists */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Prototype", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PrototypeCloak;

	/** Prototype cold chest armor marker */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Prototype", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PrototypeChestPlate;

	/** Prototype Vael star medallion marker */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Prototype", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PrototypeMedallion;

	UPROPERTY()
	UMaterialInterface* PrototypeShapeMaterial;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Light attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input|Ironcliffe")
	UInputAction* LightAttackAction;

	/** Heavy attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input|Ironcliffe")
	UInputAction* HeavyAttackAction;

	/** Parry Input Action */
	UPROPERTY(EditAnywhere, Category = "Input|Ironcliffe")
	UInputAction* ParryAction;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, Category = "Input|Ironcliffe")
	UInputAction* DodgeAction;

public:

	/** Constructor */
	AIroncliffeCharacter();	

protected:

	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void LightAttackInput();
	void HeavyAttackInput();
	void ParryInput();
	void DodgeInput();
	void OrderAttackInput();
	void OrderHoldInput();
	void OrderFlankInput();
	void OrderRetreatInput();
	void RaidWestmirInput();
	void NegotiateWestmirInput();
	void UpgradeLonghouseInput();
	void AdvanceCastleInput();
	void AdvanceDayInput();
	void ToggleStrategicMapInput();
	void InteractInput();

	class UIroncliffeWorldSubsystem* GetIroncliffeWorldState() const;
	bool TryHandleStrategicMapClick();
	FName GetStrategicCommandLocationId() const;
	FString GetStrategicCommandLocationName() const;
	void ShowPrototypeMessage(const FString& Message) const;
	void DrawCombatCue(const FColor& Color, float Reach, float Size) const;
	void ApplySiriusPrototypeLook();
	void ApplyPrototypeShapeColor(UStaticMeshComponent* Component, const FLinearColor& Color) const;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Routes Sirius' fast combo strike into the combat component */
	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	virtual bool DoLightAttack();

	/** Routes Sirius' block-breaking strike into the combat component */
	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	virtual bool DoHeavyAttack();

	/** Opens the narrow parry window from the GDD combat loop */
	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	virtual bool DoParry();

	/** Executes a stamina-driven dodge */
	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	virtual bool DoDodge();

	/** Sets the active squad command for the Alt command wheel */
	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	virtual void SetTacticalOrder(EIroncliffeTacticalOrder NewOrder);

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns IroncliffeCombatComponent subobject **/
	FORCEINLINE UIroncliffeCombatComponent* GetCombatComponent() const { return CombatComponent; }
};


