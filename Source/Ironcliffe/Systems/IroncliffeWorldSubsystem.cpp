// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/IroncliffeWorldSubsystem.h"

namespace
{
	void GetCastlePhaseCost(EIroncliffeCastlePhase Phase, int32& OutGold, int32& OutTimber, int32& OutStone, int32& OutSupplies)
	{
		const int32 CostScale = FMath::Max(1, static_cast<int32>(static_cast<uint8>(Phase)));
		OutGold = CostScale * 30;
		OutTimber = CostScale * 20;
		OutStone = CostScale * 35;
		OutSupplies = CostScale * 4;
	}

	int32 GetRequiredControlledCitiesForPhase(EIroncliffeCastlePhase Phase)
	{
		switch (Phase)
		{
		case EIroncliffeCastlePhase::Foundation:
			return 1;
		case EIroncliffeCastlePhase::Walls:
			return 2;
		case EIroncliffeCastlePhase::Towers:
			return 3;
		case EIroncliffeCastlePhase::Castle:
			return 4;
		case EIroncliffeCastlePhase::Coronation:
			return 6;
		default:
			return 0;
		}
	}
}

void UIroncliffeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SeedLocations();
	SeedBuildings();
	Castle.Phase = EIroncliffeCastlePhase::None;
	Castle.ArchitectStyle = FText::FromString(TEXT("Unchosen"));
}

bool UIroncliffeWorldSubsystem::DiscoverLocation(FName LocationId)
{
	FIroncliffeLocationState* Location = FindLocation(LocationId);
	if (!Location || Location->bDiscovered)
	{
		return false;
	}

	Location->bDiscovered = true;
	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::RaidLocation(FName LocationId)
{
	FIroncliffeLocationState* Location = FindLocation(LocationId);
	if (!Location || Location->bControlled)
	{
		return false;
	}

	const int32 SupplyCost = FMath::Clamp(Location->Garrison * 2, 8, 30);
	if (!SpendResources(0, 0, 0, SupplyCost))
	{
		return false;
	}

	Location->bControlled = true;
	Location->bTakenByForce = true;
	Location->bDiscovered = true;
	ForceVictories++;

	Resources.Gold += 35 + Location->Garrison * 4;
	Resources.Stone += 18;
	Resources.Renown += 12;
	DiscoverConnectedLocations(*Location);
	AddRumorWeight(EIroncliffeRumorTrigger::SiegeWon, 32);
	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::NegotiateLocation(FName LocationId)
{
	FIroncliffeLocationState* Location = FindLocation(LocationId);
	if (!Location || Location->bControlled)
	{
		return false;
	}

	const int32 GoldCost = FMath::Clamp(80 - Location->Diplomacy, 25, 75);
	if (!SpendResources(GoldCost, 0, 0, 6))
	{
		return false;
	}

	Location->bControlled = true;
	Location->bTakenByForce = false;
	Location->bDiscovered = true;
	VoluntaryAlliances++;

	Resources.Renown += 8;
	DiscoverConnectedLocations(*Location);
	AddRumorWeight(EIroncliffeRumorTrigger::TimePassed, 12);
	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::UpgradeBuilding(EIroncliffeBuildingType BuildingType)
{
	FIroncliffeBuildingState* Building = FindBuilding(BuildingType);
	if (!Building || !Building->bUnlocked || Building->Level >= 3)
	{
		return false;
	}

	const int32 NextLevel = Building->Level + 1;
	if (!SpendResources(NextLevel * 20, NextLevel * 18, NextLevel * 14, 0))
	{
		return false;
	}

	Building->Level = NextLevel;
	Resources.Renown += 4;

	if (BuildingType == EIroncliffeBuildingType::Market)
	{
		Resources.Gold += 20 * NextLevel;
		AddRumorWeight(EIroncliffeRumorTrigger::MarketOpened, 24);
	}
	else
	{
		AddRumorWeight(EIroncliffeRumorTrigger::BuildingRaised, 16);
	}

	if (BuildingType == EIroncliffeBuildingType::Longhouse && Building->Level >= 3)
	{
		if (FIroncliffeBuildingState* Tower = FindBuilding(EIroncliffeBuildingType::ChroniclerTower))
		{
			Tower->bUnlocked = true;
		}
	}

	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::AdvanceCastlePhase(FText ArchitectStyle)
{
	if (Castle.Phase == EIroncliffeCastlePhase::Coronation)
	{
		return false;
	}

	if (Castle.Phase == EIroncliffeCastlePhase::None)
	{
		const bool bHasControlledLocation = Locations.ContainsByPredicate([](const FIroncliffeLocationState& Location)
		{
			return Location.bControlled;
		});

		if (!bHasControlledLocation)
		{
			return false;
		}
	}

	const uint8 NextPhaseIndex = static_cast<uint8>(Castle.Phase) + 1;
	const EIroncliffeCastlePhase NextPhase = static_cast<EIroncliffeCastlePhase>(NextPhaseIndex);
	if (GetControlledCityCount() < GetRequiredControlledCitiesForPhase(NextPhase))
	{
		return false;
	}

	int32 GoldCost = 0;
	int32 TimberCost = 0;
	int32 StoneCost = 0;
	int32 SuppliesCost = 0;
	GetCastlePhaseCost(NextPhase, GoldCost, TimberCost, StoneCost, SuppliesCost);
	if (!SpendResources(GoldCost, TimberCost, StoneCost, SuppliesCost))
	{
		return false;
	}

	Castle.Phase = NextPhase;
	Castle.Progress = static_cast<int32>(NextPhaseIndex) * 100 / 6;

	if (Castle.Phase == EIroncliffeCastlePhase::Foundation && !ArchitectStyle.IsEmpty())
	{
		Castle.ArchitectStyle = ArchitectStyle;
	}

	Resources.Renown += 18;
	AddRumorWeight(EIroncliffeRumorTrigger::BuildingRaised, 20);
	BroadcastWorldChanged();
	return true;
}

void UIroncliffeWorldSubsystem::AdvanceDay()
{
	CurrentDay++;
	Resources.Gold += 6 + DailyGoldBonus;
	ApplyDailyLocationProduction();
	ProcessPendingRumors();
	AddRumorWeight(EIroncliffeRumorTrigger::TimePassed, 8);

	for (FIroncliffeRumorVisitor& Visitor : Visitors)
	{
		if (Visitor.State == EIroncliffeCompanionState::WaitingAtGate)
		{
			Visitor.DaysRemaining--;
			if (Visitor.DaysRemaining <= 0)
			{
				Visitor.State = Visitor.bJoinsEnemyIfRejected ? EIroncliffeCompanionState::JoinedEnemy : EIroncliffeCompanionState::LeftOnOwn;
				if (Visitor.State == EIroncliffeCompanionState::JoinedEnemy)
				{
					Resources.Renown = FMath::Max(0, Resources.Renown - 6);
				}
			}
		}
	}

	BroadcastWorldChanged();
}

bool UIroncliffeWorldSubsystem::AcceptVisitor(FName VisitorId)
{
	FIroncliffeRumorVisitor* Visitor = FindVisitor(VisitorId);
	if (!Visitor || Visitor->State != EIroncliffeCompanionState::WaitingAtGate)
	{
		return false;
	}

	Visitor->State = EIroncliffeCompanionState::Recruited;
	ApplyVisitorBenefit(*Visitor);
	Resources.Renown += 10;
	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::RejectVisitor(FName VisitorId)
{
	FIroncliffeRumorVisitor* Visitor = FindVisitor(VisitorId);
	if (!Visitor || Visitor->State != EIroncliffeCompanionState::WaitingAtGate)
	{
		return false;
	}

	Visitor->State = Visitor->bJoinsEnemyIfRejected ? EIroncliffeCompanionState::JoinedEnemy : EIroncliffeCompanionState::Rejected;
	AddRumorWeight(EIroncliffeRumorTrigger::TimePassed, 6);
	if (Visitor->State == EIroncliffeCompanionState::JoinedEnemy)
	{
		Resources.Renown = FMath::Max(0, Resources.Renown - 8);
	}
	BroadcastWorldChanged();
	return true;
}

bool UIroncliffeWorldSubsystem::BetrayCompanion(FName VisitorId)
{
	FIroncliffeRumorVisitor* Visitor = FindVisitor(VisitorId);
	if (!Visitor || Visitor->State != EIroncliffeCompanionState::Recruited)
	{
		return false;
	}

	Visitor->State = EIroncliffeCompanionState::Betrayed;
	CompanionBetrayals++;
	Resources.Gold += 90;
	Resources.Renown += 16;
	AddRumorWeight(EIroncliffeRumorTrigger::CompanionBetrayed, 40);
	BroadcastWorldChanged();
	return true;
}

void UIroncliffeWorldSubsystem::AddRumorWeight(EIroncliffeRumorTrigger Trigger, int32 Weight)
{
	RumorWeight += Weight;
	if (RumorWeight >= 30)
	{
		RumorWeight = 0;
		SpawnVisitorForTrigger(Trigger);
	}
}

int32 UIroncliffeWorldSubsystem::GetControlledCityCount() const
{
	int32 Count = 0;
	for (const FIroncliffeLocationState& Location : Locations)
	{
		if (Location.Type == EIroncliffeLocationType::City && Location.bControlled)
		{
			++Count;
		}
	}
	return Count;
}

int32 UIroncliffeWorldSubsystem::GetTotalCityCount() const
{
	int32 Count = 0;
	for (const FIroncliffeLocationState& Location : Locations)
	{
		if (Location.Type == EIroncliffeLocationType::City)
		{
			++Count;
		}
	}
	return Count;
}

FString UIroncliffeWorldSubsystem::GetCastleAdvanceSummary() const
{
	if (Castle.Phase == EIroncliffeCastlePhase::Coronation)
	{
		return TEXT("Castle complete: coronation reached.");
	}

	const EIroncliffeCastlePhase NextPhase = static_cast<EIroncliffeCastlePhase>(static_cast<uint8>(Castle.Phase) + 1);
	const FString PhaseName = StaticEnum<EIroncliffeCastlePhase>()->GetNameStringByValue(static_cast<int64>(NextPhase));

	int32 GoldCost = 0;
	int32 TimberCost = 0;
	int32 StoneCost = 0;
	int32 SuppliesCost = 0;
	GetCastlePhaseCost(NextPhase, GoldCost, TimberCost, StoneCost, SuppliesCost);

	if (NextPhase == EIroncliffeCastlePhase::CliffClaimed)
	{
		return FString::Printf(TEXT("Next %s: control any location, pay %dG %dW %dS %dSup"), *PhaseName, GoldCost, TimberCost, StoneCost, SuppliesCost);
	}

	const int32 RequiredCities = GetRequiredControlledCitiesForPhase(NextPhase);
	return FString::Printf(
		TEXT("Next %s: cities %d/%d, pay %dG %dW %dS %dSup"),
		*PhaseName,
		GetControlledCityCount(),
		RequiredCities,
		GoldCost,
		TimberCost,
		StoneCost,
		SuppliesCost);
}

EIroncliffeEndingPath UIroncliffeWorldSubsystem::GetProjectedEnding() const
{
	if (CompanionBetrayals > 0)
	{
		return EIroncliffeEndingPath::Shadow;
	}

	if (ForceVictories >= 3)
	{
		return EIroncliffeEndingPath::Conqueror;
	}

	if (VoluntaryAlliances >= 2)
	{
		return EIroncliffeEndingPath::Ally;
	}

	return EIroncliffeEndingPath::Undecided;
}

void UIroncliffeWorldSubsystem::SeedLocations()
{
	Locations.Reset();

	const auto AddLocation = [this](
		FName Id,
		const TCHAR* Name,
		EIroncliffeLocationType Type,
		FVector2D MapPosition,
		const TCHAR* Role,
		const TCHAR* StrategicNote,
		int32 Garrison,
		int32 Diplomacy,
		int32 DailyGold,
		int32 DailyTimber,
		int32 DailyStone,
		int32 DailySupplies,
		bool bHidden,
		std::initializer_list<FName> Connected)
	{
		FIroncliffeLocationState Location;
		Location.Id = Id;
		Location.DisplayName = FText::FromString(Name);
		Location.Role = FText::FromString(Role);
		Location.StrategicNote = FText::FromString(StrategicNote);
		Location.Type = Type;
		Location.MapPosition = MapPosition;
		Location.Garrison = Garrison;
		Location.Diplomacy = Diplomacy;
		Location.DailyGold = DailyGold;
		Location.DailyTimber = DailyTimber;
		Location.DailyStone = DailyStone;
		Location.DailySupplies = DailySupplies;
		Location.bHiddenUntilDiscovered = bHidden;
		Location.bDiscovered = !bHidden || Type == EIroncliffeLocationType::Castle || Id == TEXT("westmir") || Id == TEXT("veldhem");
		Location.ConnectedLocations = TArray<FName>(Connected);
		Locations.Add(Location);
	};

	AddLocation(TEXT("castle_rock"), TEXT("Скала Ваэля"), EIroncliffeLocationType::Castle, FVector2D(0.50f, 0.48f), TEXT("Замок-цель"), TEXT("Цель всей игры. Постройка замка - финальный акт истории."), 0, 100, 0, 0, 0, 0, false, { TEXT("veldhem"), TEXT("ashfen"), TEXT("olmir"), TEXT("brogmor") });

	AddLocation(TEXT("westmir"), TEXT("Вестмир"), EIroncliffeLocationType::City, FVector2D(0.14f, 0.13f), TEXT("Торговый город у западной реки"), TEXT("Слабый гарнизон - лучший первый захват. Мало крови, много золота."), 7, 42, 18, 4, 0, 6, false, { TEXT("dorn"), TEXT("grey_holm"), TEXT("tolvek"), TEXT("olmir") });
	AddLocation(TEXT("dret"), TEXT("Дрет"), EIroncliffeLocationType::City, FVector2D(0.86f, 0.13f), TEXT("Военный форпост у перевала"), TEXT("Брать последним. Осада без элитной армии - больно и дорого."), 18, 18, 10, 0, 8, 2, false, { TEXT("fell"), TEXT("rendal"), TEXT("varna"), TEXT("kaerhol") });
	AddLocation(TEXT("serholt"), TEXT("Серхолт"), EIroncliffeLocationType::City, FVector2D(0.06f, 0.48f), TEXT("Горнодобывающий город"), TEXT("Главный источник камня и железа. Без него замок строится как сон на кредитке."), 11, 28, 8, 0, 20, 0, false, { TEXT("maric"), TEXT("tolvek"), TEXT("aske"), TEXT("morvat") });
	AddLocation(TEXT("kaerhol"), TEXT("Каэрхол"), EIroncliffeLocationType::City, FVector2D(0.93f, 0.48f), TEXT("Перевальный город и таможня"), TEXT("Открывает снабжение к Дрету. Дипломатия выгоднее осады."), 10, 34, 22, 0, 0, 8, false, { TEXT("beorn"), TEXT("rendal"), TEXT("kazlven"), TEXT("dret") });
	AddLocation(TEXT("haldren"), TEXT("Халдрен"), EIroncliffeLocationType::City, FVector2D(0.14f, 0.83f), TEXT("Политический центр региона"), TEXT("Ключ к мирной концовке. Добровольное признание при высокой славе."), 13, 26, 25, 10, 0, 15, false, { TEXT("tirn"), TEXT("aske"), TEXT("solholt"), TEXT("morvat") });
	AddLocation(TEXT("fenmark"), TEXT("Фенмарк"), EIroncliffeLocationType::City, FVector2D(0.86f, 0.83f), TEXT("Портовый город и контрабанда"), TEXT("Уникальные слухи и темные связи. Иврат может перекрыть снабжение."), 9, 45, 20, 0, 0, 10, false, { TEXT("sorn"), TEXT("ivrat"), TEXT("colmir"), TEXT("kazlven") });

	AddLocation(TEXT("olmir"), TEXT("Олмир"), EIroncliffeLocationType::Village, FVector2D(0.41f, 0.10f), TEXT("Тихая северная деревня"), TEXT("Источник странников системы слухов. Марит может прийти отсюда."), 4, 55, 5, 2, 0, 4, false, { TEXT("castle_rock"), TEXT("westmir"), TEXT("veldhem"), TEXT("grey_holm") });
	AddLocation(TEXT("ashfen"), TEXT("Эшфен"), EIroncliffeLocationType::Village, FVector2D(0.59f, 0.10f), TEXT("Деревня кузнецов"), TEXT("Отсюда родом изгнанные мастера и редкие материалы."), 8, 44, 5, 3, 2, 3, false, { TEXT("castle_rock"), TEXT("dret"), TEXT("veldhem"), TEXT("varna") });
	AddLocation(TEXT("tolvek"), TEXT("Толвек"), EIroncliffeLocationType::Village, FVector2D(0.27f, 0.33f), TEXT("Деревня у западной реки"), TEXT("Первая точка найма рабочих для фундамента замка."), 5, 50, 4, 4, 0, 6, false, { TEXT("westmir"), TEXT("serholt"), TEXT("maric"), TEXT("castle_rock") });
	AddLocation(TEXT("rendal"), TEXT("Рендал"), EIroncliffeLocationType::Village, FVector2D(0.73f, 0.33f), TEXT("Деревня у перевала"), TEXT("Захват ослабляет Дрет и дает плацдарм для наступления."), 12, 35, 6, 2, 2, 4, false, { TEXT("dret"), TEXT("kaerhol"), TEXT("beorn"), TEXT("castle_rock") });
	AddLocation(TEXT("morvat"), TEXT("Морват"), EIroncliffeLocationType::Village, FVector2D(0.27f, 0.66f), TEXT("Болотная деревня"), TEXT("Знахари и редкие ингредиенты для лечебницы."), 4, 56, 3, 3, 0, 5, false, { TEXT("serholt"), TEXT("haldren"), TEXT("aske"), TEXT("tirn") });
	AddLocation(TEXT("kazlven"), TEXT("Казлвен"), EIroncliffeLocationType::Village, FVector2D(0.73f, 0.66f), TEXT("Богатая восточная деревня"), TEXT("Ярмарка и торговые слухи. Хорошее место для давления на Фенмарк."), 4, 58, 4, 2, 0, 5, false, { TEXT("kaerhol"), TEXT("fenmark"), TEXT("colmir"), TEXT("sorn") });
	AddLocation(TEXT("solholt"), TEXT("Сольхолт"), EIroncliffeLocationType::Village, FVector2D(0.41f, 0.88f), TEXT("Южная болотная деревня"), TEXT("Дешевые склады и мутные сделки рядом с Халдреном."), 5, 50, 6, 2, 0, 4, false, { TEXT("haldren"), TEXT("brogmor"), TEXT("tirn"), TEXT("ivrat") });
	AddLocation(TEXT("ivrat"), TEXT("Иврат"), EIroncliffeLocationType::Village, FVector2D(0.59f, 0.88f), TEXT("Рыбацкая деревня"), TEXT("Блокируя Иврат, перекрываешь снабжение Фенмарка."), 5, 52, 5, 3, 0, 5, false, { TEXT("fenmark"), TEXT("solholt"), TEXT("brogmor"), TEXT("colmir") });

	AddLocation(TEXT("dorn"), TEXT("Дорн"), EIroncliffeLocationType::Hamlet, FVector2D(0.29f, 0.20f), TEXT("Фермерское село"), TEXT("Еда для армии. Без него солдаты начинают ныть, и не без причины."), 0, 65, 1, 2, 0, 5, true, { TEXT("westmir"), TEXT("grey_holm"), TEXT("tolvek") });
	AddLocation(TEXT("fell"), TEXT("Фелл"), EIroncliffeLocationType::Hamlet, FVector2D(0.71f, 0.18f), TEXT("Горное село"), TEXT("Проводники знают тайные тропы к Дрету."), 0, 55, 1, 2, 1, 3, true, { TEXT("dret"), TEXT("ashfen"), TEXT("varna") });
	AddLocation(TEXT("maric"), TEXT("Марик"), EIroncliffeLocationType::Hamlet, FVector2D(0.18f, 0.40f), TEXT("Лесное село"), TEXT("Ранний источник дерева."), 0, 55, 1, 8, 0, 2, true, { TEXT("serholt"), TEXT("tolvek"), TEXT("westmir") });
	AddLocation(TEXT("beorn"), TEXT("Беорн"), EIroncliffeLocationType::Hamlet, FVector2D(0.82f, 0.40f), TEXT("Пограничное село"), TEXT("Признает только того, кто способен защитить."), 0, 50, 1, 2, 2, 2, true, { TEXT("kaerhol"), TEXT("rendal"), TEXT("dret") });
	AddLocation(TEXT("grey_holm"), TEXT("Грейхолм"), EIroncliffeLocationType::Hamlet, FVector2D(0.37f, 0.23f), TEXT("Старое село у руин"), TEXT("Кеор останавливался здесь. История региона в свитках."), 0, 62, 2, 3, 0, 3, true, { TEXT("westmir"), TEXT("olmir"), TEXT("dorn") });
	AddLocation(TEXT("varna"), TEXT("Варна"), EIroncliffeLocationType::Hamlet, FVector2D(0.63f, 0.22f), TEXT("Степное село"), TEXT("Место сбора наемников для первой армии."), 0, 52, 2, 2, 0, 3, true, { TEXT("ashfen"), TEXT("fell"), TEXT("dret") });
	AddLocation(TEXT("aske"), TEXT("Аске"), EIroncliffeLocationType::Hamlet, FVector2D(0.32f, 0.63f), TEXT("Болотное село"), TEXT("Травы и лечебные предметы."), 0, 56, 1, 2, 0, 3, true, { TEXT("morvat"), TEXT("haldren"), TEXT("tirn") });
	AddLocation(TEXT("colmir"), TEXT("Колмир"), EIroncliffeLocationType::Hamlet, FVector2D(0.67f, 0.62f), TEXT("Торговое село"), TEXT("Постоялый двор и восстановление на дороге к Фенмарку."), 0, 60, 2, 2, 0, 3, true, { TEXT("kazlven"), TEXT("fenmark"), TEXT("sorn") });
	AddLocation(TEXT("veldhem"), TEXT("Велдхэм"), EIroncliffeLocationType::Hamlet, FVector2D(0.50f, 0.37f), TEXT("Село под скалой"), TEXT("Первым видит строительство замка и все его последствия."), 0, 70, 1, 2, 1, 4, false, { TEXT("castle_rock"), TEXT("olmir"), TEXT("ashfen"), TEXT("grey_holm") });
	AddLocation(TEXT("brogmor"), TEXT("Брогмор"), EIroncliffeLocationType::Hamlet, FVector2D(0.50f, 0.63f), TEXT("Южное зерновое село"), TEXT("Блокада становится рычагом давления на Халдрен."), 0, 58, 1, 2, 0, 6, true, { TEXT("castle_rock"), TEXT("solholt"), TEXT("ivrat") });
	AddLocation(TEXT("tirn"), TEXT("Тирн"), EIroncliffeLocationType::Hamlet, FVector2D(0.22f, 0.75f), TEXT("Село паромщиков"), TEXT("Перекрывает снабжение Халдрена с запада."), 0, 54, 1, 3, 0, 4, true, { TEXT("haldren"), TEXT("morvat"), TEXT("aske"), TEXT("solholt") });
	AddLocation(TEXT("sorn"), TEXT("Сорн"), EIroncliffeLocationType::Hamlet, FVector2D(0.78f, 0.75f), TEXT("Село пастухов"), TEXT("Лучшие лошади региона. Будущая кавалерия."), 0, 52, 2, 2, 0, 3, true, { TEXT("fenmark"), TEXT("kazlven"), TEXT("colmir") });
}

void UIroncliffeWorldSubsystem::SeedBuildings()
{
	Buildings = {
		{ EIroncliffeBuildingType::Longhouse, FText::FromString(TEXT("Longhouse")), FText::FromString(TEXT("Settlement center; level 3 foreshadows the throne hall")), 1, true },
		{ EIroncliffeBuildingType::Barracks, FText::FromString(TEXT("Barracks")), FText::FromString(TEXT("Unlocks units; level 3 forms Vael elite guard")), 0, true },
		{ EIroncliffeBuildingType::Forge, FText::FromString(TEXT("Forge")), FText::FromString(TEXT("Repairs and craft; Oren improves it")), 0, true },
		{ EIroncliffeBuildingType::Market, FText::FromString(TEXT("Market")), FText::FromString(TEXT("Trade income and spy rumors")), 0, true },
		{ EIroncliffeBuildingType::Stables, FText::FromString(TEXT("Stables")), FText::FromString(TEXT("Scouting and cavalry reach")), 0, true },
		{ EIroncliffeBuildingType::Sanctuary, FText::FromString(TEXT("Sanctuary")), FText::FromString(TEXT("Army morale and siege blessing")), 0, true },
		{ EIroncliffeBuildingType::Infirmary, FText::FromString(TEXT("Infirmary")), FText::FromString(TEXT("Army recovery bonus unlocked through rumors")), 0, false },
		{ EIroncliffeBuildingType::ChroniclerTower, FText::FromString(TEXT("Chronicler Tower")), FText::FromString(TEXT("Unlocks the in-game third-person chronicle")), 0, false }
	};
}

void UIroncliffeWorldSubsystem::ApplyDailyLocationProduction()
{
	for (const FIroncliffeLocationState& Location : Locations)
	{
		if (!Location.bControlled)
		{
			continue;
		}

		Resources.Gold += Location.DailyGold;
		Resources.Timber += Location.DailyTimber;
		Resources.Stone += Location.DailyStone;
		Resources.Supplies += Location.DailySupplies;
	}
}

void UIroncliffeWorldSubsystem::DiscoverConnectedLocations(const FIroncliffeLocationState& Location)
{
	for (const FName& ConnectedId : Location.ConnectedLocations)
	{
		if (FIroncliffeLocationState* ConnectedLocation = FindLocation(ConnectedId))
		{
			ConnectedLocation->bDiscovered = true;
		}
	}
}

void UIroncliffeWorldSubsystem::SpawnVisitorForTrigger(EIroncliffeRumorTrigger Trigger)
{
	FIroncliffeRumorVisitor NewVisitor;
	if (!BuildVisitorForTrigger(Trigger, NewVisitor))
	{
		return;
	}

	LastPrerumorText = NewVisitor.PrerumorText;
	PendingRumors.Add({ NewVisitor, 1 });
}

bool UIroncliffeWorldSubsystem::BuildVisitorForTrigger(EIroncliffeRumorTrigger Trigger, FIroncliffeRumorVisitor& OutVisitor) const
{
	OutVisitor = FIroncliffeRumorVisitor();
	OutVisitor.Trigger = Trigger;
	OutVisitor.State = EIroncliffeCompanionState::WaitingAtGate;

	if (Trigger == EIroncliffeRumorTrigger::SiegeWon && !HasVisitorOrPending(TEXT("oren_gray")))
	{
		OutVisitor.Id = TEXT("oren_gray");
		OutVisitor.DisplayName = FText::FromString(TEXT("Орен Грей"));
		OutVisitor.Role = FText::FromString(TEXT("Изгнанный кузнец"));
		OutVisitor.Hook = FText::FromString(TEXT("Бывший придворный кузнец города, который Сириус взял силой."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("Говорят, в захваченном городе кузнец остался без хозяина. Его видели на дороге к Скале."));
		OutVisitor.GateReportText = FText::FromString(TEXT("У ворот стоит кузнец. Молот при нем, взгляд спокойный. Просит работу, а не милость."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Орен усилит кузницу и принесет чертежи для квеста 'Сломанный молот'."));
		OutVisitor.Reward = FText::FromString(TEXT("Сразу дает камень и повышает ценность кузницы."));
		OutVisitor.QuestLineId = TEXT("quest_broken_hammer");
		OutVisitor.DaysRemaining = 4;
		return true;
	}

	if (Trigger == EIroncliffeRumorTrigger::SiegeWon && !HasVisitorOrPending(TEXT("adrian_vest")))
	{
		OutVisitor.Id = TEXT("adrian_vest");
		OutVisitor.DisplayName = FText::FromString(TEXT("Сэр Адриан Вест"));
		OutVisitor.Role = FText::FromString(TEXT("Рыцарь без герба"));
		OutVisitor.Hook = FText::FromString(TEXT("Потерял прежнего господина после осады и ищет нового сюзерена."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("На дороге видели рыцаря без знамени. Едет медленно, будто выбирает судьбу."));
		OutVisitor.GateReportText = FText::FromString(TEXT("Рыцарь у ворот. Без герба. Говорит, что сила Сириуса убедительнее старой клятвы."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Адриан станет офицером и откроет ветку 'Долг чести'. Если отказать, может уйти к врагам."));
		OutVisitor.Reward = FText::FromString(TEXT("Соратник-офицер и прирост славы."));
		OutVisitor.QuestLineId = TEXT("quest_duty_of_honor");
		OutVisitor.DaysRemaining = 5;
		OutVisitor.bJoinsEnemyIfRejected = true;
		return true;
	}

	if (Trigger == EIroncliffeRumorTrigger::TimePassed && !HasVisitorOrPending(TEXT("marit_smoke")))
	{
		OutVisitor.Id = TEXT("marit_smoke");
		OutVisitor.DisplayName = FText::FromString(TEXT("Марит Дым"));
		OutVisitor.Role = FText::FromString(TEXT("Знахарка из болот"));
		OutVisitor.Hook = FText::FromString(TEXT("Лечит раненых у ворот бесплатно и говорит, что видела замок во сне."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("Жители спорят о старухе у ворот: лечит бесплатно, ничего не просит, стража нервничает."));
		OutVisitor.GateReportText = FText::FromString(TEXT("Знахарка просит аудиенции. Сидит третий день и смотрит на северное небо."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Марит откроет лечебницу и ветку пророчества о цене мечты."));
		OutVisitor.Reward = FText::FromString(TEXT("Unlocks the infirmary."));
		OutVisitor.QuestLineId = TEXT("quest_star_prophecy");
		OutVisitor.DaysRemaining = 6;
		return true;
	}

	if (Trigger == EIroncliffeRumorTrigger::BuildingRaised && !HasVisitorOrPending(TEXT("brother_keor")))
	{
		OutVisitor.Id = TEXT("brother_keor");
		OutVisitor.DisplayName = FText::FromString(TEXT("Брат Кеор"));
		OutVisitor.Role = FText::FromString(TEXT("Странствующий летописец"));
		OutVisitor.Hook = FText::FromString(TEXT("Видел пять павших королевств и хочет записать историю шестого."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("Монах в Грейхолме расспрашивал о Сириусе и записывал каждое слово."));
		OutVisitor.GateReportText = FText::FromString(TEXT("Летописец у ворот. Просит комнату, свечи и право писать правду."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Кеор открывает башню хроник и внутриигровой дневник."));
		OutVisitor.Reward = FText::FromString(TEXT("Unlocks the chronicler tower."));
		OutVisitor.QuestLineId = TEXT("quest_five_fallen_kings");
		OutVisitor.DaysRemaining = 7;
		return true;
	}

	if (Trigger == EIroncliffeRumorTrigger::MarketOpened && !HasVisitorOrPending(TEXT("tillan_spy")))
	{
		OutVisitor.Id = TEXT("tillan_spy");
		OutVisitor.DisplayName = FText::FromString(TEXT("Тиллан"));
		OutVisitor.Role = FText::FromString(TEXT("Купец, предположительно"));
		OutVisitor.Hook = FText::FromString(TEXT("Приехал с пустой телегой и слишком внимательно смотрит на башни."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("На рынке появился богатый купец без товара. Больше спрашивает про стены, чем про цены."));
		OutVisitor.GateReportText = FText::FromString(TEXT("Купец предлагает маршруты снабжения. Стража считает, что он считает башни."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Принять: золото каждый день. Риск: шпионская ветка 'Сеть'."));
		OutVisitor.Reward = FText::FromString(TEXT("Adds daily gold income."));
		OutVisitor.QuestLineId = TEXT("quest_spy_network");
		OutVisitor.DaysRemaining = 2;
		OutVisitor.bJoinsEnemyIfRejected = true;
		return true;
	}

	if (Trigger == EIroncliffeRumorTrigger::CompanionBetrayed && !HasVisitorOrPending(TEXT("unknown_shadow")))
	{
		OutVisitor.Id = TEXT("unknown_shadow");
		OutVisitor.DisplayName = FText::FromString(TEXT("Неизвестный"));
		OutVisitor.Role = FText::FromString(TEXT("???"));
		OutVisitor.Hook = FText::FromString(TEXT("Знает детали предательства, которых не должно знать никто."));
		OutVisitor.PrerumorText = FText::FromString(TEXT("Ночью кто-то прошел через ворота. Стражник не помнит, почему пропустил его."));
		OutVisitor.GateReportText = FText::FromString(TEXT("Он уже внутри. Стоит в тени и просит говорить только с Сириусом."));
		OutVisitor.OfferDescription = FText::FromString(TEXT("Открывает тайный ход и ветку 'Цена знания'."));
		OutVisitor.Reward = FText::FromString(TEXT("Secret passage branch for the castle."));
		OutVisitor.QuestLineId = TEXT("quest_price_of_knowledge");
		OutVisitor.DaysRemaining = 1;
		return true;
	}

	return false;
}

bool UIroncliffeWorldSubsystem::HasVisitorOrPending(FName VisitorId) const
{
	const bool bHasVisitor = Visitors.ContainsByPredicate([VisitorId](const FIroncliffeRumorVisitor& Visitor)
	{
		return Visitor.Id == VisitorId;
	});

	if (bHasVisitor)
	{
		return true;
	}

	return PendingRumors.ContainsByPredicate([VisitorId](const FIroncliffePendingRumor& Pending)
	{
		return Pending.Visitor.Id == VisitorId;
	});
}

void UIroncliffeWorldSubsystem::ProcessPendingRumors()
{
	for (int32 Index = PendingRumors.Num() - 1; Index >= 0; --Index)
	{
		PendingRumors[Index].DaysUntilArrival--;
		if (PendingRumors[Index].DaysUntilArrival <= 0)
		{
			Visitors.Add(PendingRumors[Index].Visitor);
			PendingRumors.RemoveAt(Index);
		}
	}
}

void UIroncliffeWorldSubsystem::ApplyVisitorBenefit(const FIroncliffeRumorVisitor& Visitor)
{
	if (Visitor.Id == TEXT("oren_gray"))
	{
		Resources.Stone += 30;
		Resources.Renown += 4;
	}
	else if (Visitor.Id == TEXT("marit_smoke"))
	{
		if (FIroncliffeBuildingState* Infirmary = FindBuilding(EIroncliffeBuildingType::Infirmary))
		{
			Infirmary->bUnlocked = true;
		}
	}
	else if (Visitor.Id == TEXT("brother_keor"))
	{
		if (FIroncliffeBuildingState* Tower = FindBuilding(EIroncliffeBuildingType::ChroniclerTower))
		{
			Tower->bUnlocked = true;
		}
	}
	else if (Visitor.Id == TEXT("tillan_spy"))
	{
		DailyGoldBonus += 25;
		Resources.Gold += 25;
	}
	else if (Visitor.Id == TEXT("adrian_vest"))
	{
		Resources.Renown += 20;
	}
	else if (Visitor.Id == TEXT("unknown_shadow"))
	{
		Castle.Progress = FMath::Min(100, Castle.Progress + 10);
	}
}

FIroncliffeLocationState* UIroncliffeWorldSubsystem::FindLocation(FName LocationId)
{
	return Locations.FindByPredicate([LocationId](const FIroncliffeLocationState& Location)
	{
		return Location.Id == LocationId;
	});
}

FIroncliffeBuildingState* UIroncliffeWorldSubsystem::FindBuilding(EIroncliffeBuildingType BuildingType)
{
	return Buildings.FindByPredicate([BuildingType](const FIroncliffeBuildingState& Building)
	{
		return Building.Type == BuildingType;
	});
}

FIroncliffeRumorVisitor* UIroncliffeWorldSubsystem::FindVisitor(FName VisitorId)
{
	return Visitors.FindByPredicate([VisitorId](const FIroncliffeRumorVisitor& Visitor)
	{
		return Visitor.Id == VisitorId;
	});
}

bool UIroncliffeWorldSubsystem::SpendResources(int32 Gold, int32 Timber, int32 Stone, int32 Supplies)
{
	if (Resources.Gold < Gold || Resources.Timber < Timber || Resources.Stone < Stone || Resources.Supplies < Supplies)
	{
		return false;
	}

	Resources.Gold -= Gold;
	Resources.Timber -= Timber;
	Resources.Stone -= Stone;
	Resources.Supplies -= Supplies;
	return true;
}

void UIroncliffeWorldSubsystem::BroadcastWorldChanged()
{
	OnWorldChanged.Broadcast();
}
