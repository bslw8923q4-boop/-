// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/IroncliffeCombatComponent.h"

UIroncliffeCombatComponent::UIroncliffeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UIroncliffeCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	Stamina = MaxStamina;
	RefreshAuthorityStance();
}

void UIroncliffeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float PreviousStamina = Stamina;
	Stamina = FMath::Min(MaxStamina, Stamina + DeltaTime * 18.0f);

	if (ComboResetTimer > 0.0f)
	{
		ComboResetTimer -= DeltaTime;
		if (ComboResetTimer <= 0.0f)
		{
			ComboStep = 0;
		}
	}

	if (ParryWindowTimer > 0.0f)
	{
		ParryWindowTimer -= DeltaTime;
	}

	if (!FMath::IsNearlyEqual(PreviousStamina, Stamina))
	{
		OnCombatChanged.Broadcast();
	}
}

bool UIroncliffeCombatComponent::LightAttack()
{
	if (!SpendStamina(12.0f))
	{
		return false;
	}

	ComboStep = (ComboStep % 3) + 1;
	ComboResetTimer = 1.6f;
	AddArmyMorale(2.0f);
	return true;
}

bool UIroncliffeCombatComponent::HeavyAttack()
{
	if (!SpendStamina(28.0f))
	{
		return false;
	}

	ComboStep = 0;
	ComboResetTimer = 0.0f;
	AddArmyMorale(5.0f);
	return true;
}

bool UIroncliffeCombatComponent::Parry()
{
	if (!SpendStamina(18.0f))
	{
		return false;
	}

	ParryWindowTimer = 0.28f;
	AddArmyMorale(8.0f);
	return true;
}

bool UIroncliffeCombatComponent::Dodge()
{
	if (!SpendStamina(24.0f))
	{
		return false;
	}

	ComboStep = 0;
	ComboResetTimer = 0.0f;
	return true;
}

void UIroncliffeCombatComponent::SetTacticalOrder(EIroncliffeTacticalOrder NewOrder)
{
	ActiveOrder = NewOrder;

	if (NewOrder == EIroncliffeTacticalOrder::HoldPosition)
	{
		AddArmyMorale(3.0f);
	}
	else if (NewOrder == EIroncliffeTacticalOrder::Retreat)
	{
		AddArmyMorale(-6.0f);
	}
	else
	{
		OnCombatChanged.Broadcast();
	}
}

void UIroncliffeCombatComponent::AddArmyMorale(float Delta)
{
	ArmyMorale = FMath::Clamp(ArmyMorale + Delta, 0.0f, 100.0f);
	RefreshAuthorityStance();
	OnCombatChanged.Broadcast();
}

void UIroncliffeCombatComponent::ApplyDamage(float Amount)
{
	Health = FMath::Clamp(Health - FMath::Max(0.0f, Amount), 0.0f, MaxHealth);
	AddArmyMorale(-Amount * 0.12f);
}

bool UIroncliffeCombatComponent::CanUseAuthorityStance() const
{
	return ArmyMorale >= AuthorityMoraleThreshold;
}

bool UIroncliffeCombatComponent::SpendStamina(float Amount)
{
	if (Stamina < Amount)
	{
		return false;
	}

	Stamina -= Amount;
	OnCombatChanged.Broadcast();
	return true;
}

void UIroncliffeCombatComponent::RefreshAuthorityStance()
{
	bAuthorityStanceActive = CanUseAuthorityStance();
}
