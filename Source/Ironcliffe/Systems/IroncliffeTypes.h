// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IroncliffeTypes.generated.h"

UENUM(BlueprintType)
enum class EIroncliffeTacticalOrder : uint8
{
	AttackTarget UMETA(DisplayName = "Attack Target"),
	HoldPosition UMETA(DisplayName = "Hold Position"),
	Flank UMETA(DisplayName = "Flank"),
	Retreat UMETA(DisplayName = "Retreat")
};

UENUM(BlueprintType)
enum class EIroncliffeBuildingType : uint8
{
	Longhouse UMETA(DisplayName = "Longhouse"),
	Barracks UMETA(DisplayName = "Barracks"),
	Forge UMETA(DisplayName = "Forge"),
	Market UMETA(DisplayName = "Market"),
	Stables UMETA(DisplayName = "Stables"),
	Sanctuary UMETA(DisplayName = "Sanctuary"),
	Infirmary UMETA(DisplayName = "Infirmary"),
	ChroniclerTower UMETA(DisplayName = "Chronicler Tower")
};

UENUM(BlueprintType)
enum class EIroncliffeCastlePhase : uint8
{
	None UMETA(DisplayName = "None"),
	CliffClaimed UMETA(DisplayName = "Cliff Claimed"),
	Foundation UMETA(DisplayName = "Foundation"),
	Walls UMETA(DisplayName = "Walls"),
	Towers UMETA(DisplayName = "Towers"),
	Castle UMETA(DisplayName = "Castle"),
	Coronation UMETA(DisplayName = "Coronation")
};

UENUM(BlueprintType)
enum class EIroncliffeRumorTrigger : uint8
{
	SiegeWon UMETA(DisplayName = "Siege Won"),
	BuildingRaised UMETA(DisplayName = "Building Raised"),
	MarketOpened UMETA(DisplayName = "Market Opened"),
	CompanionBetrayed UMETA(DisplayName = "Companion Betrayed"),
	TimePassed UMETA(DisplayName = "Time Passed")
};

UENUM(BlueprintType)
enum class EIroncliffeLocationType : uint8
{
	City UMETA(DisplayName = "City"),
	Village UMETA(DisplayName = "Village"),
	Hamlet UMETA(DisplayName = "Hamlet"),
	Castle UMETA(DisplayName = "Castle"),
	Ruin UMETA(DisplayName = "Ruin"),
	Camp UMETA(DisplayName = "Camp")
};

UENUM(BlueprintType)
enum class EIroncliffeCompanionState : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	WaitingAtGate UMETA(DisplayName = "Waiting At Gate"),
	Recruited UMETA(DisplayName = "Recruited"),
	Rejected UMETA(DisplayName = "Rejected"),
	Betrayed UMETA(DisplayName = "Betrayed"),
	LeftOnOwn UMETA(DisplayName = "Left On Own"),
	JoinedEnemy UMETA(DisplayName = "Joined Enemy")
};

UENUM(BlueprintType)
enum class EIroncliffeEndingPath : uint8
{
	Undecided UMETA(DisplayName = "Undecided"),
	Conqueror UMETA(DisplayName = "Conqueror"),
	Ally UMETA(DisplayName = "Ally"),
	Shadow UMETA(DisplayName = "Shadow")
};

USTRUCT(BlueprintType)
struct FIroncliffeResources
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Resources")
	int32 Gold = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Resources")
	int32 Timber = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Resources")
	int32 Stone = 45;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Resources")
	int32 Supplies = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Resources")
	int32 Renown = 0;
};

USTRUCT(BlueprintType)
struct FIroncliffeLocationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	FText Role;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	FText StrategicNote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	EIroncliffeLocationType Type = EIroncliffeLocationType::Hamlet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	FVector2D MapPosition = FVector2D(0.5f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 Garrison = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 Diplomacy = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	bool bControlled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	bool bTakenByForce = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	bool bDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	bool bHiddenUntilDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 DailyGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 DailyTimber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 DailyStone = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	int32 DailySupplies = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|World")
	TArray<FName> ConnectedLocations;
};

USTRUCT(BlueprintType)
struct FIroncliffeBuildingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Settlement")
	EIroncliffeBuildingType Type = EIroncliffeBuildingType::Longhouse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Settlement")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Settlement")
	FText Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Settlement", meta = (ClampMin = 0, ClampMax = 3))
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Settlement")
	bool bUnlocked = true;
};

USTRUCT(BlueprintType)
struct FIroncliffeRumorVisitor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText Role;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText Hook;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText PrerumorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText GateReportText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText OfferDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FText Reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	int32 DaysRemaining = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	EIroncliffeRumorTrigger Trigger = EIroncliffeRumorTrigger::TimePassed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FName QuestLineId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	bool bJoinsEnemyIfRejected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	bool bUniqueAppearance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	EIroncliffeCompanionState State = EIroncliffeCompanionState::Unknown;
};

USTRUCT(BlueprintType)
struct FIroncliffePendingRumor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors")
	FIroncliffeRumorVisitor Visitor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Rumors", meta = (ClampMin = 0))
	int32 DaysUntilArrival = 1;
};

USTRUCT(BlueprintType)
struct FIroncliffeCastleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Castle")
	EIroncliffeCastlePhase Phase = EIroncliffeCastlePhase::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Castle")
	FText ArchitectStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Castle")
	int32 Progress = 0;
};
