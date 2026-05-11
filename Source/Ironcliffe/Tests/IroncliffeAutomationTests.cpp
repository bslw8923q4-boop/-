// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Combat/IroncliffeCombatComponent.h"
#include "Engine/GameInstance.h"
#include "Systems/IroncliffeTypes.h"
#include "Systems/IroncliffeWorldSubsystem.h"

namespace IroncliffeTests
{
	UGameInstance* CreateTestGameInstance()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>();
		GameInstance->Init();
		return GameInstance;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIroncliffeWorldProgressionTest,
	"Ironcliffe.World.Progression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIroncliffeWorldProgressionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = IroncliffeTests::CreateTestGameInstance();
	UIroncliffeWorldSubsystem* WorldState = GameInstance->GetSubsystem<UIroncliffeWorldSubsystem>();

	TestNotNull(TEXT("World subsystem exists"), WorldState);
	if (!WorldState)
	{
		return false;
	}

	TestEqual(TEXT("Initial day"), WorldState->CurrentDay, 1);
	TestEqual(TEXT("Initial world locations"), WorldState->Locations.Num(), 27);
	TestEqual(TEXT("Initial buildings"), WorldState->Buildings.Num(), 8);
	TestFalse(TEXT("Castle cannot advance before the region is controlled"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));

	TestTrue(TEXT("Westmir can be raided with starting supplies"), WorldState->RaidLocation(TEXT("westmir")));
	TestEqual(TEXT("Force victories increment"), WorldState->ForceVictories, 1);
	TestTrue(TEXT("Raid starts a prerumor"), !WorldState->LastPrerumorText.IsEmpty());
	TestTrue(TEXT("Raid queues a visitor rumor"), WorldState->PendingRumors.Num() >= 1);
	WorldState->AdvanceDay();
	TestTrue(TEXT("Queued rumor becomes a visitor"), WorldState->Visitors.Num() >= 1);
	TestTrue(TEXT("Captured city produces daily gold"), WorldState->Resources.Gold > 143);
	TestTrue(TEXT("A second city can be negotiated from the region map"), WorldState->NegotiateLocation(TEXT("haldren")));
	TestEqual(TEXT("Voluntary alliances increment for negotiated cities"), WorldState->VoluntaryAlliances, 1);
	WorldState->AdvanceDay();
	TestTrue(TEXT("Multiple controlled cities increase the economy"), WorldState->Resources.Timber >= 10);

	const FIroncliffeLocationState* Westmir = WorldState->Locations.FindByPredicate([](const FIroncliffeLocationState& Location)
	{
		return Location.Id == TEXT("westmir");
	});
	TestNotNull(TEXT("Westmir exists"), Westmir);
	if (Westmir)
	{
		TestTrue(TEXT("Westmir is controlled"), Westmir->bControlled);
		TestTrue(TEXT("Westmir was taken by force"), Westmir->bTakenByForce);
	}

	TestTrue(TEXT("Castle cliff phase unlocks after first controlled location"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));
	TestEqual(TEXT("Castle phase advances to cliff claim"), WorldState->Castle.Phase, EIroncliffeCastlePhase::CliffClaimed);
	TestEqual(TEXT("Two cities are controlled for early castle gates"), WorldState->GetControlledCityCount(), 2);

	WorldState->Resources.Gold = 1000;
	WorldState->Resources.Timber = 1000;
	WorldState->Resources.Stone = 1000;
	WorldState->Resources.Supplies = 1000;
	TestTrue(TEXT("Foundation can be built with one controlled city"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));
	TestTrue(TEXT("Walls can be built with two controlled cities"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));
	TestFalse(TEXT("Towers require a third controlled city"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));
	TestTrue(TEXT("A third city can be negotiated for tower construction"), WorldState->NegotiateLocation(TEXT("kaerhol")));
	TestTrue(TEXT("Towers build after the third city is controlled"), WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))));

	GameInstance->Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIroncliffeRumorVisitorTest,
	"Ironcliffe.World.RumorsAndVisitors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIroncliffeRumorVisitorTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = IroncliffeTests::CreateTestGameInstance();
	UIroncliffeWorldSubsystem* WorldState = GameInstance->GetSubsystem<UIroncliffeWorldSubsystem>();

	TestNotNull(TEXT("World subsystem exists"), WorldState);
	if (!WorldState)
	{
		return false;
	}

	for (int32 DayIndex = 0; DayIndex < 5; ++DayIndex)
	{
		WorldState->AdvanceDay();
	}

	TestTrue(TEXT("Time passing attracts at least one visitor"), WorldState->Visitors.Num() >= 1);

	FIroncliffeRumorVisitor* WaitingVisitor = WorldState->Visitors.FindByPredicate([](const FIroncliffeRumorVisitor& Visitor)
	{
		return Visitor.State == EIroncliffeCompanionState::WaitingAtGate;
	});
	TestNotNull(TEXT("A visitor is waiting at the gate"), WaitingVisitor);
	if (WaitingVisitor)
	{
		const FName VisitorId = WaitingVisitor->Id;
		TestTrue(TEXT("Waiting visitor can be accepted"), WorldState->AcceptVisitor(VisitorId));

		const FIroncliffeRumorVisitor* AcceptedVisitor = WorldState->Visitors.FindByPredicate([VisitorId](const FIroncliffeRumorVisitor& Visitor)
		{
			return Visitor.Id == VisitorId;
		});
		TestNotNull(TEXT("Accepted visitor remains tracked"), AcceptedVisitor);
		if (AcceptedVisitor)
		{
			TestEqual(TEXT("Visitor state becomes recruited"), AcceptedVisitor->State, EIroncliffeCompanionState::Recruited);
		}
	}

	GameInstance->Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIroncliffeCombatVerbTest,
	"Ironcliffe.Combat.Verbs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIroncliffeCombatVerbTest::RunTest(const FString& Parameters)
{
	UIroncliffeCombatComponent* Combat = NewObject<UIroncliffeCombatComponent>();
	TestNotNull(TEXT("Combat component can be created"), Combat);
	if (!Combat)
	{
		return false;
	}

	TestEqual(TEXT("Initial stamina"), Combat->Stamina, 100.0f);
	TestTrue(TEXT("Light attack succeeds"), Combat->LightAttack());
	TestEqual(TEXT("Light attack advances combo"), Combat->ComboStep, 1);
	TestEqual(TEXT("Light attack spends stamina"), Combat->Stamina, 88.0f);

	TestTrue(TEXT("Heavy attack succeeds"), Combat->HeavyAttack());
	TestEqual(TEXT("Heavy attack resets combo"), Combat->ComboStep, 0);
	TestEqual(TEXT("Heavy attack spends stamina"), Combat->Stamina, 60.0f);

	TestTrue(TEXT("Parry succeeds"), Combat->Parry());
	TestEqual(TEXT("Parry spends stamina"), Combat->Stamina, 42.0f);

	TestTrue(TEXT("Dodge succeeds"), Combat->Dodge());
	TestEqual(TEXT("Dodge spends stamina"), Combat->Stamina, 18.0f);

	Combat->AddArmyMorale(100.0f);
	TestTrue(TEXT("Authority stance unlocks at high morale"), Combat->CanUseAuthorityStance());

	return true;
}

#endif
