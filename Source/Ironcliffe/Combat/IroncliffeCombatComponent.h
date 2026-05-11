// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/IroncliffeTypes.h"
#include "IroncliffeCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIroncliffeCombatChanged);

UCLASS(ClassGroup = (Ironcliffe), Blueprintable, meta = (BlueprintSpawnableComponent))
class IRONCLIFFE_API UIroncliffeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UIroncliffeCombatComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ironcliffe|Combat")
	FIroncliffeCombatChanged OnCombatChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float AuthorityMoraleThreshold = 75.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float Stamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float ArmyMorale = 35.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	int32 ComboStep = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	bool bAuthorityStanceActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	EIroncliffeTacticalOrder ActiveOrder = EIroncliffeTacticalOrder::AttackTarget;

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	bool LightAttack();

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	bool HeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	bool Parry();

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	bool Dodge();

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	void SetTacticalOrder(EIroncliffeTacticalOrder NewOrder);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	void AddArmyMorale(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Combat")
	bool CanUseAuthorityStance() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float ComboResetTimer = 0.0f;
	float ParryWindowTimer = 0.0f;

	bool SpendStamina(float Amount);
	void RefreshAuthorityStance();
};
