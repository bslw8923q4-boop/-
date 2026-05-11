// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/IroncliffeHUD.h"

#include "Combat/IroncliffeCombatComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "IroncliffeCharacter.h"
#include "Systems/IroncliffeWorldSubsystem.h"

namespace
{
	struct FIroncliffeMapCardSpec
	{
		FName Id;
		FString Name;
		float X;
		float Y;
		float Width;
		float Height;
	};

	TArray<FIroncliffeMapCardSpec> BuildStrategicMapCardSpecs()
	{
		return {
			{ TEXT("westmir"), TEXT("Вестмир"), 0.050f, 0.100f, 0.175f, 0.100f },
			{ TEXT("serholt"), TEXT("Серхолт"), 0.030f, 0.455f, 0.175f, 0.100f },
			{ TEXT("haldren"), TEXT("Халдрен"), 0.050f, 0.810f, 0.175f, 0.100f },
			{ TEXT("dret"), TEXT("Дрет"), 0.760f, 0.100f, 0.175f, 0.100f },
			{ TEXT("kaerhol"), TEXT("Каэрхол"), 0.780f, 0.455f, 0.175f, 0.100f },
			{ TEXT("fenmark"), TEXT("Фенмарк"), 0.760f, 0.810f, 0.175f, 0.100f }
		};
	}

	void GetStrategicMapLayout(float ScreenW, float ScreenH, float& OutX, float& OutY, float& OutW, float& OutH)
	{
		OutW = FMath::Clamp(ScreenW - 48.0f, 760.0f, 1360.0f);
		OutH = FMath::Clamp(ScreenH - 134.0f, 430.0f, 820.0f);
		OutX = (ScreenW - OutW) * 0.5f;
		OutY = 24.0f;
	}

	bool IsPointInsideRect(const FVector2D& Point, float X, float Y, float W, float H)
	{
		return Point.X >= X && Point.X <= X + W && Point.Y >= Y && Point.Y <= Y + H;
	}
}

void AIroncliffeHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	if (bStrategicMapOpen)
	{
		DrawStrategicMap();
		return;
	}

	DrawCompactHud();
}

void AIroncliffeHUD::ToggleStrategicMap()
{
	bStrategicMapOpen = !bStrategicMapOpen;
}

bool AIroncliffeHUD::SelectStrategicMapLocation(const FVector2D& ScreenPosition, FString& OutMessage)
{
	if (!bStrategicMapOpen)
	{
		OutMessage = TEXT("Strategic map is closed.");
		return false;
	}

	int32 ViewportW = 0;
	int32 ViewportH = 0;
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		PlayerController->GetViewportSize(ViewportW, ViewportH);
	}

	if (ViewportW <= 0 || ViewportH <= 0)
	{
		OutMessage = TEXT("Strategic map cannot read the viewport yet.");
		return false;
	}

	float MapX = 0.0f;
	float MapY = 0.0f;
	float MapW = 0.0f;
	float MapH = 0.0f;
	GetStrategicMapLayout(static_cast<float>(ViewportW), static_cast<float>(ViewportH), MapX, MapY, MapW, MapH);

	for (const FIroncliffeMapCardSpec& Card : BuildStrategicMapCardSpecs())
	{
		if (IsPointInsideRect(ScreenPosition, MapX + MapW * Card.X, MapY + MapH * Card.Y, MapW * Card.Width, MapH * Card.Height))
		{
			SelectedStrategicLocationId = Card.Id;
			SelectedStrategicLocationName = Card.Name;
			OutMessage = FString::Printf(TEXT("Выбран город: %s. R - рейд, N - переговоры."), *SelectedStrategicLocationName);
			return true;
		}
	}

	OutMessage = TEXT("Кликни по карточке города на карте.");
	return false;
}

void AIroncliffeHUD::DrawCompactHud()
{
	UIroncliffeWorldSubsystem* WorldState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;
	const AIroncliffeCharacter* Character = Cast<AIroncliffeCharacter>(GetOwningPawn());
	const UIroncliffeCombatComponent* Combat = Character ? Character->GetCombatComponent() : nullptr;

	const float Left = 24.0f;
	float Top = FMath::Max(72.0f, Canvas->ClipY - 150.0f);
	const FLinearColor Warm = FLinearColor(0.92f, 0.86f, 0.68f, 1.0f);
	const FLinearColor Pale = FLinearColor(0.84f, 0.82f, 0.74f, 1.0f);

	DrawText(TEXT("IRONCLIFFE"), Warm, Left, Top, nullptr, 1.05f);
	Top += 22.0f;
	DrawText(TEXT("Runtime v4 map build active"), FLinearColor(0.52f, 0.96f, 0.42f, 1.0f), Left, Top, nullptr, 0.72f);
	Top += 18.0f;

	if (Combat)
	{
		DrawText(FString::Printf(TEXT("HP %.0f | STA %.0f | Morale %.0f | Combo %d"),
			Combat->Health,
			Combat->Stamina,
			Combat->ArmyMorale,
			Combat->ComboStep), Pale, Left, Top, nullptr, 0.86f);
		Top += 18.0f;
	}

	if (WorldState)
	{
		const FIroncliffeResources& Resources = WorldState->Resources;
		const FString CastlePhase = StaticEnum<EIroncliffeCastlePhase>()->GetNameStringByValue(static_cast<int64>(WorldState->Castle.Phase));
		const FString Ending = StaticEnum<EIroncliffeEndingPath>()->GetNameStringByValue(static_cast<int64>(WorldState->GetProjectedEnding()));

		DrawText(FString::Printf(TEXT("Day %d | Gold %d | Wood %d | Stone %d | Supplies %d | Renown %d"),
			WorldState->CurrentDay,
			Resources.Gold,
			Resources.Timber,
			Resources.Stone,
			Resources.Supplies,
			Resources.Renown), Pale, Left, Top, nullptr, 0.86f);
		Top += 18.0f;

		DrawText(FString::Printf(TEXT("Castle %s | Path %s | Visitors %d"),
			*CastlePhase,
			*Ending,
			WorldState->Visitors.Num()), Pale, Left, Top, nullptr, 0.86f);
		Top += 22.0f;

		DrawText(WorldState->GetCastleAdvanceSummary(), Warm, Left, Top, nullptr, 0.68f);
		Top += 18.0f;

		if (!WorldState->LastPrerumorText.IsEmpty())
		{
			DrawText(FString::Printf(TEXT("Rumor: %s"), *WorldState->LastPrerumorText.ToString()), Warm, Left, Top, nullptr, 0.68f);
			Top += 18.0f;
		}
	}

	DrawText(TEXT("M map | E/F interact | LMB/J light | RMB/K heavy | Q parry | Shift dodge"), Warm, Left, Top, nullptr, 0.82f);
}

void AIroncliffeHUD::DrawStrategicMap()
{
	UIroncliffeWorldSubsystem* WorldState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;

	const float ScreenW = Canvas->ClipX;
	const float ScreenH = Canvas->ClipY;
	float MapX = 0.0f;
	float MapY = 0.0f;
	float MapW = 0.0f;
	float MapH = 0.0f;
	GetStrategicMapLayout(ScreenW, ScreenH, MapX, MapY, MapW, MapH);
	const float ControlY = MapY + MapH + 18.0f;

	const FLinearColor Shade(0.0f, 0.0f, 0.0f, 0.76f);
	const FLinearColor Land(0.70f, 0.82f, 0.55f, 1.0f);
	const FLinearColor Lowland(0.62f, 0.80f, 0.66f, 0.92f);
	const FLinearColor Plateau(0.78f, 0.88f, 0.61f, 0.92f);
	const FLinearColor Road(0.74f, 0.64f, 0.39f, 0.50f);
	const FLinearColor River(0.32f, 0.68f, 0.86f, 0.82f);
	const FLinearColor Ink(0.07f, 0.07f, 0.06f, 1.0f);
	const FLinearColor Muted(0.20f, 0.20f, 0.17f, 0.88f);
	const FLinearColor Castle(0.35f, 0.28f, 0.78f, 1.0f);
	const FLinearColor Pale(0.93f, 0.90f, 0.78f, 1.0f);

	auto PX = [MapX, MapW](float N) { return MapX + MapW * N; };
	auto PY = [MapY, MapH](float N) { return MapY + MapH * N; };

	auto LocationState = [WorldState](FName Id) -> const FIroncliffeLocationState*
	{
		return WorldState ? WorldState->Locations.FindByPredicate([Id](const FIroncliffeLocationState& Location)
		{
			return Location.Id == Id;
		}) : nullptr;
	};

	auto IsControlled = [&LocationState](FName Id)
	{
		const FIroncliffeLocationState* Location = LocationState(Id);
		return Location && Location->bControlled;
	};

	auto IsForceTaken = [&LocationState](FName Id)
	{
		const FIroncliffeLocationState* Location = LocationState(Id);
		return Location && Location->bControlled && Location->bTakenByForce;
	};

	DrawRect(Shade, 0.0f, 0.0f, ScreenW, ScreenH);
	DrawRect(FLinearColor(0.015f, 0.015f, 0.012f, 1.0f), 0.0f, ControlY - 10.0f, ScreenW, ScreenH - ControlY + 10.0f);
	DrawRect(Land, MapX, MapY, MapW, MapH);
	DrawRect(Plateau, MapX, MapY, MapW, MapH * 0.18f);
	DrawRect(Lowland, MapX, PY(0.83f), MapW, MapH * 0.17f);
	DrawRect(FLinearColor(0.63f, 0.76f, 0.50f, 0.36f), MapX, PY(0.16f), MapW * 0.12f, MapH * 0.80f);
	DrawRect(FLinearColor(0.58f, 0.69f, 0.52f, 0.38f), PX(0.88f), MapY, MapW * 0.12f, MapH);
	DrawRect(FLinearColor(0.88f, 0.96f, 0.70f, 0.55f), PX(0.12f), PY(0.20f), MapW * 0.76f, MapH * 0.34f);
	DrawFrame(MapX, MapY, MapW, MapH, FLinearColor(0.82f, 0.93f, 0.62f, 1.0f), 2.0f);

	DrawLine(PX(0.15f), PY(-0.04f), PX(0.18f), PY(1.00f), River, 7.0f);
	DrawLine(PX(0.83f), PY(-0.04f), PX(0.80f), PY(1.00f), River, 7.0f);
	DrawLine(PX(0.18f), PY(0.50f), PX(0.50f), PY(0.50f), River, 5.0f);
	DrawLine(PX(0.50f), PY(0.50f), PX(0.80f), PY(0.50f), River, 5.0f);

	DrawLine(PX(0.50f), PY(0.52f), PX(0.15f), PY(0.16f), Road, 4.0f);
	DrawLine(PX(0.50f), PY(0.52f), PX(0.84f), PY(0.16f), Road, 4.0f);
	DrawLine(PX(0.50f), PY(0.52f), PX(0.18f), PY(0.82f), Road, 4.0f);
	DrawLine(PX(0.50f), PY(0.52f), PX(0.82f), PY(0.82f), Road, 4.0f);
	DrawLine(PX(0.17f), PY(0.50f), PX(0.83f), PY(0.50f), Road, 3.0f);
	DrawLine(PX(0.18f), PY(0.82f), PX(0.20f), PY(0.98f), Road, 4.0f);

	if (WorldState)
	{
		for (const FIroncliffeLocationState& Location : WorldState->Locations)
		{
			const bool bLocationVisible = Location.bDiscovered || !Location.bHiddenUntilDiscovered;
			if (!bLocationVisible)
			{
				continue;
			}

			for (const FName& ConnectedId : Location.ConnectedLocations)
			{
				const FIroncliffeLocationState* ConnectedLocation = WorldState->Locations.FindByPredicate([ConnectedId](const FIroncliffeLocationState& Candidate)
				{
					return Candidate.Id == ConnectedId;
				});

				if (!ConnectedLocation || Location.Id.ToString() > ConnectedLocation->Id.ToString())
				{
					continue;
				}

				const bool bConnectedVisible = ConnectedLocation->bDiscovered || !ConnectedLocation->bHiddenUntilDiscovered;
				if (bConnectedVisible)
				{
					DrawLine(PX(Location.MapPosition.X), PY(Location.MapPosition.Y), PX(ConnectedLocation->MapPosition.X), PY(ConnectedLocation->MapPosition.Y), Road, 1.6f);
				}
			}
		}
	}

	for (const FVector2D& Forest : {
		FVector2D(0.08f, 0.25f), FVector2D(0.08f, 0.58f), FVector2D(0.24f, 0.74f),
		FVector2D(0.65f, 0.73f), FVector2D(0.86f, 0.24f), FVector2D(0.66f, 0.08f), FVector2D(0.24f, 0.08f)
	})
	{
		DrawRect(FLinearColor(0.33f, 0.62f, 0.22f, 0.72f), PX(Forest.X), PY(Forest.Y), 15.0f, 15.0f);
		DrawRect(FLinearColor(0.33f, 0.62f, 0.22f, 0.62f), PX(Forest.X + 0.015f), PY(Forest.Y + 0.012f), 15.0f, 15.0f);
		DrawRect(FLinearColor(0.33f, 0.62f, 0.22f, 0.58f), PX(Forest.X - 0.012f), PY(Forest.Y + 0.010f), 15.0f, 15.0f);
	}

	DrawCircleOutline(PX(0.08f), PY(0.33f), 38.0f, FLinearColor(0.48f, 0.66f, 0.32f, 0.42f), 2.0f, 36);
	DrawCircleOutline(PX(0.095f), PY(0.63f), 38.0f, FLinearColor(0.48f, 0.66f, 0.32f, 0.42f), 2.0f, 36);
	DrawLine(PX(0.78f), PY(0.05f), PX(0.81f), PY(0.00f), FLinearColor(0.54f, 0.56f, 0.48f, 0.70f), 22.0f);
	DrawLine(PX(0.82f), PY(0.06f), PX(0.86f), PY(0.02f), FLinearColor(0.54f, 0.56f, 0.48f, 0.70f), 22.0f);
	DrawLine(PX(0.86f), PY(0.04f), PX(0.90f), PY(0.01f), FLinearColor(0.54f, 0.56f, 0.48f, 0.70f), 22.0f);

	DrawLine(PX(0.43f), PY(0.49f), PX(0.50f), PY(0.37f), FLinearColor(0.32f, 0.31f, 0.28f, 1.0f), 5.0f);
	DrawLine(PX(0.50f), PY(0.37f), PX(0.57f), PY(0.49f), FLinearColor(0.32f, 0.31f, 0.28f, 1.0f), 5.0f);
	DrawLine(PX(0.43f), PY(0.49f), PX(0.57f), PY(0.49f), FLinearColor(0.32f, 0.31f, 0.28f, 1.0f), 5.0f);
	DrawRect(Castle, PX(0.475f), PY(0.365f), MapW * 0.05f, MapH * 0.04f);
	DrawRect(FLinearColor(0.92f, 0.93f, 0.88f, 1.0f), PX(0.481f), PY(0.374f), MapW * 0.038f, MapH * 0.025f);
	DrawLine(PX(0.50f), PY(0.36f), PX(0.50f), PY(0.32f), Castle, 3.0f);
	DrawText(TEXT("Скала Ваэля"), Castle, PX(0.43f), PY(0.51f), nullptr, 1.06f);
	DrawText(TEXT("цель игры"), FLinearColor(0.42f, 0.34f, 0.90f, 1.0f), PX(0.455f), PY(0.54f), nullptr, 0.82f);

	DrawCircleOutline(PX(0.91f), PY(0.065f), 30.0f, FLinearColor(0.92f, 0.93f, 0.86f, 0.95f), 2.0f, 32);
	DrawText(TEXT("C"), Muted, PX(0.91f) - 5.0f, PY(0.065f) - 23.0f, nullptr, 0.70f);
	DrawText(TEXT("B"), Muted, PX(0.91f) + 17.0f, PY(0.065f) - 5.0f, nullptr, 0.70f);
	DrawText(TEXT("3"), Muted, PX(0.91f) - 24.0f, PY(0.065f) - 5.0f, nullptr, 0.70f);
	DrawText(TEXT("Ю"), Muted, PX(0.91f) - 5.0f, PY(0.065f) + 16.0f, nullptr, 0.70f);

	DrawStrategicCard(TEXT("Вестмир"), TEXT("Торговый · слабый гарнизон"), TEXT("Север-запад"), PX(0.045f), PY(0.105f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.04f, 0.36f, 0.68f, 1.0f), IsControlled(TEXT("westmir")), IsForceTaken(TEXT("westmir")), SelectedStrategicLocationId == TEXT("westmir"));
	DrawStrategicCard(TEXT("Серхолт"), TEXT("Горнодобыв. · железо"), TEXT("Запад"), PX(0.035f), PY(0.455f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.32f, 0.27f, 0.75f, 1.0f), IsControlled(TEXT("serholt")), IsForceTaken(TEXT("serholt")), SelectedStrategicLocationId == TEXT("serholt"));
	DrawStrategicCard(TEXT("Халдрен"), TEXT("Политич. центр · крупный"), TEXT("Юг-запад"), PX(0.045f), PY(0.815f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.18f, 0.43f, 0.03f, 1.0f), IsControlled(TEXT("haldren")), IsForceTaken(TEXT("haldren")), SelectedStrategicLocationId == TEXT("haldren"));
	DrawStrategicCard(TEXT("Дрет"), TEXT("Военный · сильный гарнизон"), TEXT("Север-восток"), PX(0.765f), PY(0.105f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.62f, 0.16f, 0.06f, 1.0f), IsControlled(TEXT("dret")), IsForceTaken(TEXT("dret")), SelectedStrategicLocationId == TEXT("dret"));
	DrawStrategicCard(TEXT("Каэрхол"), TEXT("Перевальный · таможня"), TEXT("Восток"), PX(0.785f), PY(0.455f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.58f, 0.34f, 0.02f, 1.0f), IsControlled(TEXT("kaerhol")), IsForceTaken(TEXT("kaerhol")), SelectedStrategicLocationId == TEXT("kaerhol"));
	DrawStrategicCard(TEXT("Фенмарк"), TEXT("Портовый · контрабанда"), TEXT("Юг-восток"), PX(0.765f), PY(0.815f), MapW * 0.17f, MapH * 0.10f, FLinearColor(0.48f, 0.10f, 0.23f, 1.0f), IsControlled(TEXT("fenmark")), IsForceTaken(TEXT("fenmark")), SelectedStrategicLocationId == TEXT("fenmark"));

	if (WorldState)
	{
		for (const FIroncliffeLocationState& Location : WorldState->Locations)
		{
			if (Location.Type == EIroncliffeLocationType::City || Location.Type == EIroncliffeLocationType::Castle)
			{
				continue;
			}

			const float NodeX = PX(Location.MapPosition.X);
			const float NodeY = PY(Location.MapPosition.Y);
			const bool bVisible = Location.bDiscovered || !Location.bHiddenUntilDiscovered;
			const FString DisplayName = bVisible ? Location.DisplayName.ToString() : FString(TEXT("?"));

			if (Location.Type == EIroncliffeLocationType::Village)
			{
				DrawMapNode(DisplayName, bVisible ? TEXT("деревня") : TEXT("неизвестно"), NodeX, NodeY, 28.0f, bVisible ? Muted : FLinearColor(0.20f, 0.20f, 0.18f, 0.52f));
			}
			else if (Location.Type == EIroncliffeLocationType::Hamlet)
			{
				const FLinearColor HamletColor = bVisible ? FLinearColor(0.48f, 0.48f, 0.45f, 0.95f) : FLinearColor(0.22f, 0.22f, 0.20f, 0.45f);
				DrawCircleOutline(NodeX, NodeY, 10.0f, HamletColor, 2.0f, 18);
				DrawText(DisplayName, bVisible ? Muted : FLinearColor(0.18f, 0.18f, 0.16f, 0.70f), NodeX - 18.0f, NodeY + 10.0f, nullptr, 0.60f);
			}
		}
	}

	DrawLegendItem(TEXT("Город"), MapX + 18.0f, ControlY + 8.0f, FLinearColor(0.04f, 0.36f, 0.68f, 1.0f));
	DrawLegendItem(TEXT("Деревня"), MapX + 126.0f, ControlY + 8.0f, FLinearColor(0.30f, 0.30f, 0.27f, 1.0f));
	DrawLegendItem(TEXT("Село"), MapX + 260.0f, ControlY + 8.0f, FLinearColor(0.72f, 0.70f, 0.64f, 1.0f));
	DrawLegendItem(TEXT("Замок-цель"), MapX + 370.0f, ControlY + 8.0f, Castle);
	DrawLegendItem(TEXT("Реки"), MapX + 535.0f, ControlY + 8.0f, River);
	DrawText(TEXT("Нажми на любое место"), FLinearColor(0.62f, 0.60f, 0.54f, 1.0f), MapX + 650.0f, ControlY + 2.0f, nullptr, 0.66f);
	DrawText(TEXT("для выбора города"), FLinearColor(0.62f, 0.60f, 0.54f, 1.0f), MapX + 650.0f, ControlY + 22.0f, nullptr, 0.66f);

	const float ButtonY = ControlY + 48.0f;
	DrawMapButton(TEXT("Лор городов ↗"), MapX + MapW * 0.34f, ButtonY, 220.0f, 44.0f, false);
	DrawMapButton(TEXT("Захват городов ↗"), MapX + MapW * 0.53f, ButtonY, 250.0f, 44.0f, true);
	DrawText(FString::Printf(TEXT("Выбран: %s | R рейд | N переговоры | M закрыть"), *SelectedStrategicLocationName), Pale, MapX + 18.0f, ButtonY + 12.0f, nullptr, 0.72f);

	if (WorldState)
	{
		DrawText(FString::Printf(TEXT("День %d | золото %d | слава %d | у ворот %d | слухи %d"),
			WorldState->CurrentDay,
			WorldState->Resources.Gold,
			WorldState->Resources.Renown,
			WorldState->Visitors.Num(),
			WorldState->PendingRumors.Num()), Pale, MapX + MapW - 420.0f, ControlY + 24.0f, nullptr, 0.76f);
	}
}

void AIroncliffeHUD::DrawStrategicCard(const FString& Title, const FString& Descriptor, const FString& Direction, float X, float Y, float Width, float Height, const FLinearColor& Accent, bool bControlled, bool bTakenByForce, bool bSelected)
{
	const FLinearColor Fill = bControlled ? FLinearColor(Accent.R * 0.28f + 0.12f, Accent.G * 0.28f + 0.16f, Accent.B * 0.28f + 0.12f, 0.92f) : FLinearColor(0.96f, 0.95f, 0.88f, 0.94f);
	const FLinearColor Label = bControlled ? FLinearColor(0.95f, 0.90f, 0.70f, 1.0f) : Accent;
	const FString State = bControlled ? (bTakenByForce ? TEXT("захвачен") : TEXT("союз")) : TEXT("не подчинён");

	DrawRect(Fill, X, Y, Width, Height);
	if (bSelected)
	{
		DrawFrame(X - 5.0f, Y - 5.0f, Width + 10.0f, Height + 10.0f, FLinearColor(0.97f, 0.88f, 0.44f, 1.0f), 4.0f);
	}
	DrawFrame(X, Y, Width, Height, Accent, 3.0f);
	DrawRect(Accent, X, Y, Width, 30.0f);
	DrawText(Title, FLinearColor::White, X + 14.0f, Y + 7.0f, nullptr, 0.82f);
	DrawText(Descriptor, FLinearColor(0.13f, 0.12f, 0.10f, 1.0f), X + 10.0f, Y + 40.0f, nullptr, 0.64f);
	DrawText(Direction, Label, X + 46.0f, Y + 64.0f, nullptr, 0.64f);
	DrawText(State, Label, X + Width - 92.0f, Y + Height - 22.0f, nullptr, 0.55f);
}

void AIroncliffeHUD::DrawMapNode(const FString& Title, const FString& Subtitle, float X, float Y, float Radius, const FLinearColor& Accent)
{
	DrawCircleOutline(X, Y, Radius, Accent, 2.0f, 32);
	DrawRect(FLinearColor(0.92f, 0.91f, 0.86f, 0.90f), X - Radius + 4.0f, Y - 10.0f, Radius * 2.0f - 8.0f, 20.0f);
	DrawText(Title, Accent, X - Radius + 10.0f, Y - 12.0f, nullptr, 0.68f);
	DrawText(Subtitle, FLinearColor(0.34f, 0.34f, 0.31f, 1.0f), X - Radius + 12.0f, Y + 10.0f, nullptr, 0.55f);
}

void AIroncliffeHUD::DrawLegendItem(const FString& Label, float X, float Y, const FLinearColor& Color)
{
	DrawRect(Color, X, Y + 8.0f, 15.0f, 15.0f);
	DrawText(Label, FLinearColor(0.76f, 0.75f, 0.68f, 1.0f), X + 24.0f, Y, nullptr, 0.76f);
}

void AIroncliffeHUD::DrawMapButton(const FString& Label, float X, float Y, float Width, float Height, bool bPrimary)
{
	const FLinearColor Fill = bPrimary ? FLinearColor(0.06f, 0.05f, 0.035f, 0.92f) : FLinearColor(0.02f, 0.02f, 0.018f, 0.92f);
	const FLinearColor Border = bPrimary ? FLinearColor(0.95f, 0.93f, 0.84f, 1.0f) : FLinearColor(0.78f, 0.76f, 0.68f, 1.0f);
	DrawRect(Fill, X, Y, Width, Height);
	DrawFrame(X, Y, Width, Height, Border, 2.0f);
	DrawText(Label, FLinearColor(0.95f, 0.93f, 0.84f, 1.0f), X + 28.0f, Y + 11.0f, nullptr, 0.86f);
}

void AIroncliffeHUD::DrawCircleOutline(float CenterX, float CenterY, float Radius, const FLinearColor& Color, float Thickness, int32 Segments)
{
	const int32 SafeSegments = FMath::Max(8, Segments);
	float LastX = CenterX + Radius;
	float LastY = CenterY;

	for (int32 Index = 1; Index <= SafeSegments; ++Index)
	{
		const float Angle = static_cast<float>(Index) / static_cast<float>(SafeSegments) * UE_TWO_PI;
		const float NextX = CenterX + FMath::Cos(Angle) * Radius;
		const float NextY = CenterY + FMath::Sin(Angle) * Radius;
		DrawLine(LastX, LastY, NextX, NextY, Color, Thickness);
		LastX = NextX;
		LastY = NextY;
	}
}

void AIroncliffeHUD::DrawFrame(float X, float Y, float Width, float Height, const FLinearColor& Color, float Thickness)
{
	DrawLine(X, Y, X + Width, Y, Color, Thickness);
	DrawLine(X + Width, Y, X + Width, Y + Height, Color, Thickness);
	DrawLine(X + Width, Y + Height, X, Y + Height, Color, Thickness);
	DrawLine(X, Y + Height, X, Y, Color, Thickness);
}
