// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/IroncliffeTypes.h"
#include "IroncliffeWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIroncliffeWorldChanged);

UCLASS()
class IRONCLIFFE_API UIroncliffeWorldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Ironcliffe|World")
	FIroncliffeWorldChanged OnWorldChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|World")
	int32 CurrentDay = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|World")
	FIroncliffeResources Resources;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|World")
	TArray<FIroncliffeLocationState> Locations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Settlement")
	TArray<FIroncliffeBuildingState> Buildings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Castle")
	FIroncliffeCastleState Castle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Rumors")
	TArray<FIroncliffeRumorVisitor> Visitors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Rumors")
	TArray<FIroncliffePendingRumor> PendingRumors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Rumors")
	FText LastPrerumorText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Economy")
	int32 DailyGoldBonus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Endings")
	int32 ForceVictories = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Endings")
	int32 VoluntaryAlliances = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Endings")
	int32 CompanionBetrayals = 0;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|World")
	bool RaidLocation(FName LocationId);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|World")
	bool NegotiateLocation(FName LocationId);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Settlement")
	bool UpgradeBuilding(EIroncliffeBuildingType BuildingType);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Castle")
	bool AdvanceCastlePhase(FText ArchitectStyle);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Rumors")
	void AdvanceDay();

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Rumors")
	bool AcceptVisitor(FName VisitorId);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Rumors")
	bool RejectVisitor(FName VisitorId);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Rumors")
	bool BetrayCompanion(FName VisitorId);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Rumors")
	void AddRumorWeight(EIroncliffeRumorTrigger Trigger, int32 Weight);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|World")
	bool DiscoverLocation(FName LocationId);

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|World")
	int32 GetControlledCityCount() const;

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|World")
	int32 GetTotalCityCount() const;

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Castle")
	FString GetCastleAdvanceSummary() const;

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Endings")
	EIroncliffeEndingPath GetProjectedEnding() const;

private:
	int32 RumorWeight = 0;

	void SeedLocations();
	void SeedBuildings();
	void ApplyDailyLocationProduction();
	void DiscoverConnectedLocations(const FIroncliffeLocationState& Location);
	void SpawnVisitorForTrigger(EIroncliffeRumorTrigger Trigger);
	bool BuildVisitorForTrigger(EIroncliffeRumorTrigger Trigger, FIroncliffeRumorVisitor& OutVisitor) const;
	bool HasVisitorOrPending(FName VisitorId) const;
	void ProcessPendingRumors();
	void ApplyVisitorBenefit(const FIroncliffeRumorVisitor& Visitor);
	FIroncliffeLocationState* FindLocation(FName LocationId);
	FIroncliffeBuildingState* FindBuilding(EIroncliffeBuildingType BuildingType);
	FIroncliffeRumorVisitor* FindVisitor(FName VisitorId);
	bool SpendResources(int32 Gold, int32 Timber, int32 Stone, int32 Supplies);
	void BroadcastWorldChanged();
};
