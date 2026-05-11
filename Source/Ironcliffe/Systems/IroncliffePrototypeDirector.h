// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/IroncliffeInteractableActor.h"
#include "IroncliffePrototypeDirector.generated.h"

class UTextRenderComponent;
class UStaticMesh;
class UMaterialInterface;
class UIroncliffeWorldSubsystem;

UCLASS()
class IRONCLIFFE_API AIroncliffePrototypeDirector : public AActor
{
	GENERATED_BODY()

public:
	AIroncliffePrototypeDirector();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	USceneComponent* SceneRoot;

	UPROPERTY()
	UTextRenderComponent* StatusText;

	UPROPERTY()
	UTextRenderComponent* WestmirLabel;

	UPROPERTY()
	UTextRenderComponent* LonghouseLabel;

	UPROPERTY()
	UTextRenderComponent* CastleLabel;

	UPROPERTY()
	UStaticMeshComponent* WestmirMarker;

	UPROPERTY()
	UStaticMeshComponent* LonghouseMesh;

	UPROPERTY()
	UStaticMeshComponent* LonghouseRoof;

	UPROPERTY()
	UStaticMeshComponent* ForgeMesh;

	UPROPERTY()
	UStaticMeshComponent* MarketMesh;

	UPROPERTY()
	UStaticMeshComponent* CastleFoundation;

	UPROPERTY()
	UStaticMeshComponent* CastleWallNorth;

	UPROPERTY()
	UStaticMeshComponent* CastleWallSouth;

	UPROPERTY()
	UStaticMeshComponent* CastleWallWest;

	UPROPERTY()
	UStaticMeshComponent* CastleWallEast;

	UPROPERTY()
	UStaticMeshComponent* CastleGatehouse;

	UPROPERTY()
	UStaticMeshComponent* CastleKeep;

	UPROPERTY()
	UStaticMeshComponent* CastleInnerHall;

	UPROPERTY()
	UStaticMeshComponent* CastleTowerWest;

	UPROPERTY()
	UStaticMeshComponent* CastleTowerEast;

	UPROPERTY()
	UStaticMeshComponent* CastleTowerNorth;

	UPROPERTY()
	UStaticMeshComponent* CastleTowerSouth;

	UPROPERTY()
	UStaticMeshComponent* VaelFlagPole;

	UPROPERTY()
	UStaticMeshComponent* VaelFlag;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> VisitorMarkers;

	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> DynamicLabels;

	UPROPERTY()
	TMap<FName, TObjectPtr<UStaticMeshComponent>> LocationMarkers;

	UPROPERTY()
	TMap<FName, TObjectPtr<UTextRenderComponent>> LocationLabels;

	UPROPERTY()
	UStaticMesh* CubeMesh;

	UPROPERTY()
	UStaticMesh* CylinderMesh;

	UPROPERTY()
	UStaticMesh* ConeMesh;

	UPROPERTY()
	UStaticMesh* SphereMesh;

	UPROPERTY()
	UMaterialInterface* ShapeMaterial;

	UPROPERTY()
	UMaterialInterface* LavaMaterial;

	UPROPERTY()
	UMaterialInterface* RuinMaterial;

	UPROPERTY()
	UIroncliffeWorldSubsystem* WorldSubsystem;

	void ClearTemplateBlockout();
	void TuneLighting();
	void PlacePlayerForPrototype();
	void BuildPrototypeScene();

	UFUNCTION()
	void RefreshStatusText();
	void RefreshWorldVisuals();

	UStaticMeshComponent* AddShape(FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator, const FLinearColor& Color = FLinearColor(0.32f, 0.30f, 0.27f, 1.0f), UMaterialInterface* OverrideMaterial = nullptr);
	UTextRenderComponent* AddLabel(FName Name, const FString& Text, const FVector& Location, float Size = 34.0f, const FColor& Color = FColor::White);
	void AddInteractable(FName Name, const FVector& Location, EIroncliffeInteractionType InteractionType, const FString& Prompt, const FString& SuccessMessage, const FString& FailureMessage, FName LocationId = NAME_None, EIroncliffeBuildingType BuildingType = EIroncliffeBuildingType::Longhouse);
};
