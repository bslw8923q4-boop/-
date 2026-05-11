// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/IroncliffePrototypeDirector.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Ironcliffe.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Systems/IroncliffeInteractableActor.h"
#include "Systems/IroncliffeWorldSubsystem.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	void SetShapeColor(UStaticMeshComponent* Component, const FLinearColor& Color)
	{
		if (!Component)
		{
			return;
		}

		if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Component->GetMaterial(0)))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
	}
}

AIroncliffePrototypeDirector::AIroncliffePrototypeDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LavaMaterialFinder(TEXT("/Game/Variant_Combat/Materials/M_Lava.M_Lava"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RuinMaterialFinder(TEXT("/Game/Variant_Combat/Materials/MI_Box_Destroyed.MI_Box_Destroyed"));

	CubeMesh = CubeFinder.Object;
	CylinderMesh = CylinderFinder.Object;
	ConeMesh = ConeFinder.Object;
	SphereMesh = SphereFinder.Object;
	ShapeMaterial = MaterialFinder.Object;
	LavaMaterial = LavaMaterialFinder.Object;
	RuinMaterial = RuinMaterialFinder.Object;
}

void AIroncliffePrototypeDirector::BeginPlay()
{
	Super::BeginPlay();

	WorldSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;
	if (WorldSubsystem)
	{
		WorldSubsystem->OnWorldChanged.AddDynamic(this, &AIroncliffePrototypeDirector::RefreshStatusText);
	}

	ClearTemplateBlockout();
	TuneLighting();
	BuildPrototypeScene();
	PlacePlayerForPrototype();
	RefreshStatusText();
}

void AIroncliffePrototypeDirector::ClearTemplateBlockout()
{
	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		AStaticMeshActor* MeshActor = *It;
		if (MeshActor)
		{
			MeshActor->Destroy();
		}
	}
}

void AIroncliffePrototypeDirector::TuneLighting()
{
	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		if (UDirectionalLightComponent* LightComponent = Cast<UDirectionalLightComponent>(It->GetLightComponent()))
		{
			LightComponent->SetIntensity(2.0f);
			LightComponent->SetLightColor(FLinearColor(1.0f, 0.86f, 0.64f));
		}
	}

	for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
	{
		if (UExponentialHeightFogComponent* FogComponent = It->GetComponent())
		{
			FogComponent->SetFogDensity(0.035f);
			FogComponent->SetFogHeightFalloff(0.18f);
			FogComponent->SetFogInscatteringColor(FLinearColor(0.18f, 0.22f, 0.24f));
		}
	}
}

void AIroncliffePrototypeDirector::PlacePlayerForPrototype()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	PlayerPawn->SetActorLocation(FVector(-720.0f, -620.0f, 110.0f));
	PlayerPawn->SetActorRotation(FRotator(0.0f, 38.0f, 0.0f));

	if (AController* Controller = PlayerPawn->GetController())
	{
		Controller->SetControlRotation(FRotator(-8.0f, 38.0f, 0.0f));
	}
}

void AIroncliffePrototypeDirector::BuildPrototypeScene()
{
	AddShape(TEXT("PrototypeGround"), CubeMesh, FVector(0.0f, 0.0f, -28.0f), FVector(22.0f, 22.0f, 0.28f), FRotator::ZeroRotator, FLinearColor(0.11f, 0.105f, 0.09f, 1.0f));
	AddShape(TEXT("MistValley"), CubeMesh, FVector(260.0f, 100.0f, -2.0f), FVector(4.2f, 7.5f, 0.08f), FRotator(0.0f, -18.0f, 0.0f), FLinearColor(0.18f, 0.23f, 0.22f, 1.0f));
	AddLabel(TEXT("Title"), TEXT("IRONCLIFFE\nprice of the dream"), FVector(-840.0f, -690.0f, 210.0f), 30.0f, FColor(232, 218, 178));

	// Vael's cliff and castle silhouette.
	AddShape(TEXT("CliffBase"), CylinderMesh, FVector(520.0f, 120.0f, 85.0f), FVector(2.6f, 2.6f, 1.7f), FRotator::ZeroRotator, FLinearColor(0.24f, 0.24f, 0.22f, 1.0f));
	AddShape(TEXT("CliffPeak"), ConeMesh, FVector(520.0f, 120.0f, 265.0f), FVector(2.9f, 2.9f, 1.9f), FRotator::ZeroRotator, FLinearColor(0.20f, 0.205f, 0.195f, 1.0f));
	CastleFoundation = AddShape(TEXT("CastleFoundation"), CubeMesh, FVector(520.0f, 120.0f, 390.0f), FVector(1.8f, 1.15f, 0.28f), FRotator::ZeroRotator, FLinearColor(0.38f, 0.36f, 0.31f, 1.0f));
	CastleWallNorth = AddShape(TEXT("CastleWallNorth"), CubeMesh, FVector(520.0f, -15.0f, 455.0f), FVector(2.30f, 0.14f, 0.55f), FRotator::ZeroRotator, FLinearColor(0.35f, 0.35f, 0.32f, 1.0f));
	CastleWallSouth = AddShape(TEXT("CastleWallSouth"), CubeMesh, FVector(520.0f, 255.0f, 455.0f), FVector(2.30f, 0.14f, 0.55f), FRotator::ZeroRotator, FLinearColor(0.35f, 0.35f, 0.32f, 1.0f));
	CastleWallWest = AddShape(TEXT("CastleWallWest"), CubeMesh, FVector(285.0f, 120.0f, 455.0f), FVector(0.14f, 1.50f, 0.55f), FRotator::ZeroRotator, FLinearColor(0.35f, 0.35f, 0.32f, 1.0f));
	CastleWallEast = AddShape(TEXT("CastleWallEast"), CubeMesh, FVector(755.0f, 120.0f, 455.0f), FVector(0.14f, 1.50f, 0.55f), FRotator::ZeroRotator, FLinearColor(0.35f, 0.35f, 0.32f, 1.0f));
	CastleGatehouse = AddShape(TEXT("CastleGatehouse"), CubeMesh, FVector(520.0f, -32.0f, 485.0f), FVector(0.82f, 0.32f, 0.72f), FRotator::ZeroRotator, FLinearColor(0.30f, 0.30f, 0.28f, 1.0f));
	CastleInnerHall = AddShape(TEXT("CastleInnerHall"), CubeMesh, FVector(520.0f, 120.0f, 478.0f), FVector(1.18f, 0.78f, 0.70f), FRotator::ZeroRotator, FLinearColor(0.29f, 0.285f, 0.265f, 1.0f));
	CastleKeep = AddShape(TEXT("CastleKeep"), CubeMesh, FVector(520.0f, 120.0f, 480.0f), FVector(0.72f, 0.72f, 1.3f), FRotator::ZeroRotator, FLinearColor(0.30f, 0.30f, 0.28f, 1.0f));
	CastleTowerWest = AddShape(TEXT("CastleTowerWest"), CylinderMesh, FVector(390.0f, 20.0f, 500.0f), FVector(0.44f, 0.44f, 1.65f), FRotator::ZeroRotator, FLinearColor(0.31f, 0.31f, 0.29f, 1.0f));
	CastleTowerEast = AddShape(TEXT("CastleTowerEast"), CylinderMesh, FVector(650.0f, 220.0f, 500.0f), FVector(0.44f, 0.44f, 1.65f), FRotator::ZeroRotator, FLinearColor(0.31f, 0.31f, 0.29f, 1.0f));
	CastleTowerNorth = AddShape(TEXT("CastleTowerNorth"), CylinderMesh, FVector(650.0f, 20.0f, 500.0f), FVector(0.38f, 0.38f, 1.35f), FRotator::ZeroRotator, FLinearColor(0.31f, 0.31f, 0.29f, 1.0f));
	CastleTowerSouth = AddShape(TEXT("CastleTowerSouth"), CylinderMesh, FVector(390.0f, 220.0f, 500.0f), FVector(0.38f, 0.38f, 1.35f), FRotator::ZeroRotator, FLinearColor(0.31f, 0.31f, 0.29f, 1.0f));
	VaelFlagPole = AddShape(TEXT("VaelFlagPole"), CubeMesh, FVector(520.0f, 120.0f, 650.0f), FVector(0.05f, 0.05f, 1.6f), FRotator::ZeroRotator, FLinearColor(0.09f, 0.08f, 0.075f, 1.0f));
	VaelFlag = AddShape(TEXT("VaelFlag"), CubeMesh, FVector(565.0f, 120.0f, 720.0f), FVector(0.9f, 0.04f, 0.32f), FRotator::ZeroRotator, FLinearColor(0.58f, 0.10f, 0.08f, 1.0f));
	CastleLabel = AddLabel(TEXT("CastleLabel"), TEXT("Vael's Cliff\nE: castle phase"), FVector(325.0f, -85.0f, 565.0f), 22.0f, FColor(200, 167, 90));
	AddInteractable(
		TEXT("CastleInteractable"),
		FVector(520.0f, 120.0f, 58.0f),
		EIroncliffeInteractionType::AdvanceCastle,
		TEXT("advance castle phase"),
		TEXT("Castle phase advanced. The cliff remembers the price."),
		TEXT("Castle blocked: take a location first or gather resources."));

	// Settlement hub.
	AddShape(TEXT("GateLeft"), CubeMesh, FVector(-350.0f, 150.0f, 90.0f), FVector(0.45f, 0.45f, 1.8f), FRotator::ZeroRotator, FLinearColor(0.18f, 0.16f, 0.13f, 1.0f));
	AddShape(TEXT("GateRight"), CubeMesh, FVector(-160.0f, 150.0f, 90.0f), FVector(0.45f, 0.45f, 1.8f), FRotator::ZeroRotator, FLinearColor(0.18f, 0.16f, 0.13f, 1.0f));
	AddShape(TEXT("GateCrossbeam"), CubeMesh, FVector(-255.0f, 150.0f, 205.0f), FVector(2.4f, 0.34f, 0.26f), FRotator::ZeroRotator, FLinearColor(0.18f, 0.16f, 0.13f, 1.0f));
	AddLabel(TEXT("RumorGateLabel"), TEXT("Rumor Gate\nE: accept visitor"), FVector(-465.0f, 105.0f, 235.0f), 22.0f, FColor(170, 203, 148));
	AddInteractable(
		TEXT("RumorGateInteractable"),
		FVector(-255.0f, 150.0f, 45.0f),
		EIroncliffeInteractionType::AcceptFirstVisitor,
		TEXT("accept waiting visitor"),
		TEXT("Visitor accepted. Another soul tied to the dream."),
		TEXT("No visitor waits at the gate yet. Let rumors build."));

	LonghouseMesh = AddShape(TEXT("Longhouse"), CubeMesh, FVector(-360.0f, -170.0f, 55.0f), FVector(1.7f, 0.85f, 0.75f), FRotator::ZeroRotator, FLinearColor(0.30f, 0.22f, 0.16f, 1.0f));
	LonghouseRoof = AddShape(TEXT("LonghouseRoof"), ConeMesh, FVector(-360.0f, -170.0f, 140.0f), FVector(1.35f, 0.65f, 0.52f), FRotator(0.0f, 45.0f, 0.0f), FLinearColor(0.16f, 0.10f, 0.08f, 1.0f));
	LonghouseLabel = AddLabel(TEXT("LonghouseLabel"), TEXT("Longhouse\nE: upgrade"), FVector(-515.0f, -250.0f, 174.0f), 21.0f, FColor(232, 232, 220));
	AddInteractable(
		TEXT("LonghouseInteractable"),
		FVector(-360.0f, -170.0f, 45.0f),
		EIroncliffeInteractionType::UpgradeBuilding,
		TEXT("upgrade longhouse"),
		TEXT("Longhouse upgraded. Vael's camp hardens into rule."),
		TEXT("Longhouse upgrade blocked: resources or max level."),
		NAME_None,
		EIroncliffeBuildingType::Longhouse);

	ForgeMesh = AddShape(TEXT("Forge"), CubeMesh, FVector(-120.0f, -220.0f, 48.0f), FVector(0.9f, 0.65f, 0.65f), FRotator::ZeroRotator, FLinearColor(0.24f, 0.18f, 0.16f, 1.0f));
	AddShape(TEXT("ForgeChimney"), CubeMesh, FVector(-70.0f, -180.0f, 125.0f), FVector(0.18f, 0.18f, 1.0f), FRotator::ZeroRotator, FLinearColor(0.10f, 0.09f, 0.085f, 1.0f));
	AddLabel(TEXT("ForgeLabel"), TEXT("Forge\nE: upgrade"), FVector(-205.0f, -330.0f, 145.0f), 19.0f, FColor(220, 128, 100));
	AddInteractable(
		TEXT("ForgeInteractable"),
		FVector(-120.0f, -220.0f, 45.0f),
		EIroncliffeInteractionType::UpgradeBuilding,
		TEXT("upgrade forge"),
		TEXT("Forge upgraded. Steel answers ambition."),
		TEXT("Forge upgrade blocked: resources, unlocks, or max level."),
		NAME_None,
		EIroncliffeBuildingType::Forge);

	MarketMesh = AddShape(TEXT("Market"), CubeMesh, FVector(-560.0f, 20.0f, 42.0f), FVector(1.0f, 0.75f, 0.46f), FRotator::ZeroRotator, FLinearColor(0.30f, 0.25f, 0.13f, 1.0f));
	AddShape(TEXT("MarketCanopy"), CubeMesh, FVector(-560.0f, 20.0f, 100.0f), FVector(1.25f, 0.9f, 0.15f), FRotator::ZeroRotator, FLinearColor(0.54f, 0.36f, 0.10f, 1.0f));
	AddLabel(TEXT("MarketLabel"), TEXT("Market\nE: upgrade"), FVector(-705.0f, -35.0f, 142.0f), 19.0f, FColor(222, 194, 96));
	AddInteractable(
		TEXT("MarketInteractable"),
		FVector(-560.0f, 20.0f, 45.0f),
		EIroncliffeInteractionType::UpgradeBuilding,
		TEXT("upgrade market"),
		TEXT("Market upgraded. Coin carries rumors farther."),
		TEXT("Market upgrade blocked: resources or max level."),
		NAME_None,
		EIroncliffeBuildingType::Market);

	// Strategic region map built from ready engine meshes/materials.
	const FVector RegionCenter(520.0f, 120.0f, 0.0f);
	const FVector2D CastleMapPosition(0.50f, 0.48f);
	auto MapToWorld = [RegionCenter, CastleMapPosition](const FVector2D& MapPosition, float Z)
	{
		return FVector(
			RegionCenter.X + (MapPosition.X - CastleMapPosition.X) * 1800.0f,
			RegionCenter.Y + (MapPosition.Y - CastleMapPosition.Y) * 1550.0f,
			Z);
	};

	auto AddSegment = [this](const FString& Name, const FVector& Start, const FVector& End, float Width, float Z, const FLinearColor& Color)
	{
		const FVector Delta(End.X - Start.X, End.Y - Start.Y, 0.0f);
		const float Length = Delta.Size();
		if (Length < 1.0f)
		{
			return;
		}

		const FVector Midpoint((Start.X + End.X) * 0.5f, (Start.Y + End.Y) * 0.5f, Z);
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
		AddShape(*Name, CubeMesh, Midpoint, FVector(Length / 100.0f, Width / 100.0f, 0.035f), FRotator(0.0f, Yaw, 0.0f), Color);
	};

	auto AddTree = [this](const FString& Name, const FVector& Location, float Scale)
	{
		AddShape(*FString::Printf(TEXT("%sTrunk"), *Name), CylinderMesh, Location + FVector(0.0f, 0.0f, 26.0f * Scale), FVector(0.10f * Scale, 0.10f * Scale, 0.52f * Scale), FRotator::ZeroRotator, FLinearColor(0.16f, 0.10f, 0.055f, 1.0f));
		AddShape(*FString::Printf(TEXT("%sCrown"), *Name), ConeMesh, Location + FVector(0.0f, 0.0f, 86.0f * Scale), FVector(0.34f * Scale, 0.34f * Scale, 0.72f * Scale), FRotator::ZeroRotator, FLinearColor(0.16f, 0.34f, 0.13f, 1.0f));
	};

	auto AddHouse = [this](const FString& Name, const FVector& Location, float Scale, const FLinearColor& WallColor)
	{
		AddShape(*FString::Printf(TEXT("%sBody"), *Name), CubeMesh, Location + FVector(0.0f, 0.0f, 26.0f * Scale), FVector(0.34f * Scale, 0.26f * Scale, 0.28f * Scale), FRotator::ZeroRotator, WallColor);
		AddShape(*FString::Printf(TEXT("%sRoof"), *Name), ConeMesh, Location + FVector(0.0f, 0.0f, 68.0f * Scale), FVector(0.30f * Scale, 0.22f * Scale, 0.26f * Scale), FRotator(0.0f, 45.0f, 0.0f), FLinearColor(0.19f, 0.09f, 0.055f, 1.0f));
	};

	AddShape(TEXT("RegionGroundReadyAssetPass"), CubeMesh, RegionCenter + FVector(0.0f, 90.0f, -42.0f), FVector(24.0f, 19.0f, 0.18f), FRotator::ZeroRotator, FLinearColor(0.20f, 0.29f, 0.16f, 1.0f));
	AddShape(TEXT("RegionNorthHighlands"), CubeMesh, RegionCenter + FVector(0.0f, -620.0f, -30.0f), FVector(23.4f, 4.2f, 0.08f), FRotator(0.0f, -4.0f, 0.0f), FLinearColor(0.33f, 0.43f, 0.23f, 1.0f));
	AddShape(TEXT("RegionSouthMarsh"), CubeMesh, RegionCenter + FVector(40.0f, 690.0f, -29.0f), FVector(23.0f, 4.5f, 0.08f), FRotator(0.0f, 3.0f, 0.0f), FLinearColor(0.18f, 0.34f, 0.28f, 1.0f));
	AddShape(TEXT("RegionEastPass"), CubeMesh, RegionCenter + FVector(840.0f, 35.0f, -28.0f), FVector(2.6f, 15.0f, 0.07f), FRotator(0.0f, -4.0f, 0.0f), FLinearColor(0.28f, 0.27f, 0.21f, 1.0f));
	AddLabel(TEXT("RegionMapLabel"), TEXT("IRONCLIFFE REGION\nasset map pass"), RegionCenter + FVector(-1030.0f, -760.0f, 118.0f), 22.0f, FColor(235, 220, 170));

	const TArray<FVector> WestRiver = {
		MapToWorld(FVector2D(0.10f, 0.00f), 5.0f),
		MapToWorld(FVector2D(0.13f, 0.25f), 5.0f),
		MapToWorld(FVector2D(0.13f, 0.49f), 5.0f),
		MapToWorld(FVector2D(0.17f, 0.72f), 5.0f),
		MapToWorld(FVector2D(0.18f, 1.00f), 5.0f)
	};
	const TArray<FVector> EastRiver = {
		MapToWorld(FVector2D(0.84f, 0.00f), 6.0f),
		MapToWorld(FVector2D(0.82f, 0.22f), 6.0f),
		MapToWorld(FVector2D(0.83f, 0.48f), 6.0f),
		MapToWorld(FVector2D(0.80f, 0.72f), 6.0f),
		MapToWorld(FVector2D(0.81f, 1.00f), 6.0f)
	};
	for (int32 Index = 1; Index < WestRiver.Num(); ++Index)
	{
		AddSegment(FString::Printf(TEXT("WestRiver%02d"), Index), WestRiver[Index - 1], WestRiver[Index], 36.0f, 8.0f, FLinearColor(0.14f, 0.45f, 0.62f, 1.0f));
		AddSegment(FString::Printf(TEXT("EastRiver%02d"), Index), EastRiver[Index - 1], EastRiver[Index], 36.0f, 8.0f, FLinearColor(0.14f, 0.45f, 0.62f, 1.0f));
	}
	AddSegment(TEXT("CentralTradeRoad"), MapToWorld(FVector2D(0.08f, 0.48f), 12.0f), MapToWorld(FVector2D(0.92f, 0.48f), 12.0f), 22.0f, 13.0f, FLinearColor(0.53f, 0.45f, 0.27f, 1.0f));

	for (int32 Index = 0; Index < 18; ++Index)
	{
		const float XOffset = static_cast<float>((Index % 6) * 56);
		const float YOffset = static_cast<float>((Index / 6) * 52);
		AddTree(FString::Printf(TEXT("WestForestTree%02d"), Index), MapToWorld(FVector2D(0.06f, 0.30f), 16.0f) + FVector(XOffset, YOffset, 0.0f), 0.72f + 0.05f * (Index % 3));
		AddTree(FString::Printf(TEXT("SouthForestTree%02d"), Index), MapToWorld(FVector2D(0.58f, 0.73f), 16.0f) + FVector(XOffset * 0.82f, YOffset, 0.0f), 0.68f + 0.04f * (Index % 4));
	}

	for (int32 Index = 0; Index < 9; ++Index)
	{
		AddShape(*FString::Printf(TEXT("DretMountain%02d"), Index), ConeMesh, MapToWorld(FVector2D(0.88f, 0.04f), 72.0f) + FVector((Index % 3) * 96.0f, (Index / 3) * 74.0f, 0.0f), FVector(0.70f, 0.70f, 1.10f + 0.14f * (Index % 2)), FRotator::ZeroRotator, FLinearColor(0.31f, 0.32f, 0.30f, 1.0f));
	}

	TMap<FName, FVector> LocationWorldPositions;
	if (WorldSubsystem)
	{
		for (const FIroncliffeLocationState& Location : WorldSubsystem->Locations)
		{
			LocationWorldPositions.Add(Location.Id, MapToWorld(Location.MapPosition, 34.0f));
		}

		TSet<FString> DrawnRoads;
		for (const FIroncliffeLocationState& Location : WorldSubsystem->Locations)
		{
			const FVector* From = LocationWorldPositions.Find(Location.Id);
			if (!From)
			{
				continue;
			}

			for (const FName& ConnectedId : Location.ConnectedLocations)
			{
				const FVector* To = LocationWorldPositions.Find(ConnectedId);
				if (!To)
				{
					continue;
				}

				const FString ThisId = Location.Id.ToString();
				const FString OtherId = ConnectedId.ToString();
				const bool bThisFirst = ThisId.Compare(OtherId, ESearchCase::CaseSensitive) < 0;
				const FString A = bThisFirst ? ThisId : OtherId;
				const FString B = bThisFirst ? OtherId : ThisId;
				const FString RoadKey = FString::Printf(TEXT("%s_%s"), *A, *B);
				if (DrawnRoads.Contains(RoadKey))
				{
					continue;
				}

				DrawnRoads.Add(RoadKey);
				AddSegment(FString::Printf(TEXT("Road_%s"), *RoadKey), *From + FVector(0.0f, 0.0f, -2.0f), *To + FVector(0.0f, 0.0f, -2.0f), 14.0f, 16.0f, FLinearColor(0.59f, 0.49f, 0.28f, 1.0f));
			}
		}

		for (const FIroncliffeLocationState& Location : WorldSubsystem->Locations)
		{
			const FVector* Position = LocationWorldPositions.Find(Location.Id);
			if (!Position)
			{
				continue;
			}

			const FString Id = Location.Id.ToString();
			const bool bControlled = Location.bControlled;
			const bool bLocationHidden = Location.bHiddenUntilDiscovered && !Location.bDiscovered;
			const FLinearColor CityColor = bControlled ? FLinearColor(0.16f, 0.55f, 0.24f, 1.0f) : FLinearColor(0.11f, 0.36f, 0.64f, 1.0f);
			const FLinearColor VillageColor = bLocationHidden ? FLinearColor(0.28f, 0.28f, 0.25f, 1.0f) : FLinearColor(0.70f, 0.66f, 0.56f, 1.0f);

			if (Location.Type == EIroncliffeLocationType::City)
			{
				UStaticMeshComponent* CityBase = AddShape(*FString::Printf(TEXT("City_%s_Base"), *Id), CubeMesh, *Position + FVector(0.0f, 0.0f, 30.0f), FVector(0.72f, 0.54f, 0.30f), FRotator::ZeroRotator, CityColor);
				AddShape(*FString::Printf(TEXT("City_%s_TowerA"), *Id), CylinderMesh, *Position + FVector(-36.0f, -24.0f, 92.0f), FVector(0.16f, 0.16f, 0.72f), FRotator::ZeroRotator, FLinearColor(0.27f, 0.27f, 0.25f, 1.0f));
				AddShape(*FString::Printf(TEXT("City_%s_TowerB"), *Id), CylinderMesh, *Position + FVector(38.0f, 26.0f, 82.0f), FVector(0.14f, 0.14f, 0.60f), FRotator::ZeroRotator, FLinearColor(0.27f, 0.27f, 0.25f, 1.0f));
				for (int32 GuardIndex = 0; GuardIndex < FMath::Clamp(Location.Garrison / 4, 1, 5); ++GuardIndex)
				{
					AddShape(*FString::Printf(TEXT("City_%s_Garrison_%02d"), *Id, GuardIndex), ConeMesh, *Position + FVector(-62.0f + GuardIndex * 28.0f, 64.0f, 44.0f), FVector(0.10f, 0.10f, 0.36f), FRotator::ZeroRotator, FLinearColor(0.58f, 0.12f, 0.10f, 1.0f));
				}

				UTextRenderComponent* CityLabel = AddLabel(*FString::Printf(TEXT("City_%s_Label"), *Id), FString::Printf(TEXT("%s\nG%d D%d"), *Location.DisplayName.ToString(), Location.Garrison, Location.Diplomacy), *Position + FVector(-56.0f, -45.0f, 112.0f), 15.0f, FColor(210, 226, 232));
				LocationMarkers.Add(Location.Id, CityBase);
				LocationLabels.Add(Location.Id, CityLabel);

				AddInteractable(
					*FString::Printf(TEXT("Raid_%s_Interactable"), *Id),
					FVector(Position->X - 70.0f, Position->Y - 52.0f, 45.0f),
					EIroncliffeInteractionType::RaidLocation,
					FString::Printf(TEXT("raid %s"), *Location.DisplayName.ToString()),
					FString::Printf(TEXT("%s taken by force. Its resources now feed Vael's cliff."), *Location.DisplayName.ToString()),
					FString::Printf(TEXT("Raid blocked: %s needs more supplies or is already resolved."), *Location.DisplayName.ToString()),
					Location.Id);
				AddInteractable(
					*FString::Printf(TEXT("Negotiate_%s_Interactable"), *Id),
					FVector(Position->X + 70.0f, Position->Y - 52.0f, 45.0f),
					EIroncliffeInteractionType::NegotiateLocation,
					FString::Printf(TEXT("negotiate %s"), *Location.DisplayName.ToString()),
					FString::Printf(TEXT("%s accepts Vael's rule without blood."), *Location.DisplayName.ToString()),
					FString::Printf(TEXT("Negotiation blocked: %s needs coin, supplies, or is already resolved."), *Location.DisplayName.ToString()),
					Location.Id);
				if (Location.Id == TEXT("westmir"))
				{
					WestmirMarker = CityBase;
					WestmirLabel = CityLabel;
				}
			}
			else if (Location.Type == EIroncliffeLocationType::Village)
			{
				UStaticMeshComponent* VillageMarker = AddShape(*FString::Printf(TEXT("Village_%s_Marker"), *Id), SphereMesh, *Position + FVector(0.0f, 0.0f, 30.0f), FVector(0.26f, 0.26f, 0.20f), FRotator::ZeroRotator, FLinearColor(0.62f, 0.59f, 0.48f, 1.0f));
				AddHouse(FString::Printf(TEXT("Village_%s_HouseA"), *Id), *Position + FVector(-30.0f, 18.0f, 0.0f), 0.72f, VillageColor);
				AddHouse(FString::Printf(TEXT("Village_%s_HouseB"), *Id), *Position + FVector(28.0f, -18.0f, 0.0f), 0.62f, VillageColor);
				UTextRenderComponent* VillageLabel = AddLabel(*FString::Printf(TEXT("Village_%s_Label"), *Id), Location.DisplayName.ToString(), *Position + FVector(-44.0f, -34.0f, 86.0f), 11.0f, FColor(205, 204, 180));
				LocationMarkers.Add(Location.Id, VillageMarker);
				LocationLabels.Add(Location.Id, VillageLabel);
			}
			else if (Location.Type == EIroncliffeLocationType::Hamlet)
			{
				UStaticMeshComponent* HamletMarker = AddShape(*FString::Printf(TEXT("Hamlet_%s_Marker"), *Id), SphereMesh, *Position + FVector(0.0f, 0.0f, 24.0f), FVector(0.16f, 0.16f, 0.13f), FRotator::ZeroRotator, VillageColor);
				LocationMarkers.Add(Location.Id, HamletMarker);
				AddHouse(FString::Printf(TEXT("Hamlet_%s_Hut"), *Id), *Position + FVector(22.0f, 12.0f, 0.0f), 0.46f, VillageColor);
				if (!bLocationHidden)
				{
					UTextRenderComponent* HamletLabel = AddLabel(*FString::Printf(TEXT("Hamlet_%s_Label"), *Id), Location.DisplayName.ToString(), *Position + FVector(-28.0f, -24.0f, 62.0f), 8.5f, FColor(184, 184, 164));
					LocationLabels.Add(Location.Id, HamletLabel);
				}
			}
		}

		UE_LOG(LogIroncliffe, Warning, TEXT("Ironcliffe asset region map built: %d locations, %d road links"), WorldSubsystem->Locations.Num(), DrawnRoads.Num());
	}

	AddInteractable(
		TEXT("CampfireDayInteractable"),
		FVector(-660.0f, 300.0f, 42.0f),
		EIroncliffeInteractionType::AdvanceDay,
		TEXT("pass a day"),
		TEXT("A day passes. Rumors walk faster than soldiers."),
		TEXT("Time refuses to move. Weird as hell."));
	AddShape(TEXT("Campfire"), ConeMesh, FVector(-660.0f, 300.0f, 58.0f), FVector(0.36f, 0.36f, 0.72f), FRotator::ZeroRotator, FLinearColor(0.92f, 0.35f, 0.12f, 1.0f), LavaMaterial);
	AddLabel(TEXT("CampfireLabel"), TEXT("Campfire\nE: pass day"), FVector(-750.0f, 255.0f, 126.0f), 18.0f, FColor(240, 150, 86));

	StatusText = AddLabel(TEXT("StatusText"), TEXT("Loading Ironcliffe state..."), FVector(-900.0f, 470.0f, 180.0f), 16.0f, FColor(245, 239, 220));
	DynamicLabels.Add(StatusText);

	VisitorMarkers.Add(AddShape(TEXT("VisitorMarkerOne"), SphereMesh, FVector(-290.0f, 245.0f, 58.0f), FVector(0.22f, 0.22f, 0.22f), FRotator::ZeroRotator, FLinearColor(0.55f, 0.80f, 0.62f, 1.0f)));
	VisitorMarkers.Add(AddShape(TEXT("VisitorMarkerTwo"), SphereMesh, FVector(-245.0f, 250.0f, 58.0f), FVector(0.22f, 0.22f, 0.22f), FRotator::ZeroRotator, FLinearColor(0.55f, 0.80f, 0.62f, 1.0f)));
	VisitorMarkers.Add(AddShape(TEXT("VisitorMarkerThree"), SphereMesh, FVector(-200.0f, 245.0f, 58.0f), FVector(0.22f, 0.22f, 0.22f), FRotator::ZeroRotator, FLinearColor(0.55f, 0.80f, 0.62f, 1.0f)));
	RefreshWorldVisuals();
}

void AIroncliffePrototypeDirector::RefreshStatusText()
{
	if (!StatusText || !WorldSubsystem)
	{
		return;
	}

	const FIroncliffeResources& Resources = WorldSubsystem->Resources;
	const FString Ending = StaticEnum<EIroncliffeEndingPath>()->GetNameStringByValue(static_cast<int64>(WorldSubsystem->GetProjectedEnding()));
	const FString CastlePhase = StaticEnum<EIroncliffeCastlePhase>()->GetNameStringByValue(static_cast<int64>(WorldSubsystem->Castle.Phase));
	int32 ControlledCities = 0;
	int32 TotalCities = 0;
	int32 DailyGold = WorldSubsystem->DailyGoldBonus;
	int32 DailyTimber = 0;
	int32 DailyStone = 0;
	int32 DailySupplies = 0;
	for (const FIroncliffeLocationState& Location : WorldSubsystem->Locations)
	{
		if (Location.Type == EIroncliffeLocationType::City)
		{
			++TotalCities;
			ControlledCities += Location.bControlled ? 1 : 0;
		}

		if (Location.bControlled)
		{
			DailyGold += Location.DailyGold;
			DailyTimber += Location.DailyTimber;
			DailyStone += Location.DailyStone;
			DailySupplies += Location.DailySupplies;
		}
	}

	StatusText->SetText(FText::FromString(FString::Printf(
		TEXT("Day %d | Gold %d | Wood %d | Stone %d | Supplies %d\nCities %d/%d | Daily +%dG +%dW +%dS +%dSup\nCastle: %s | %s\nPath: %s | Gate %d | Rumors %d\n%s\nE/F near city markers. C builds Vael's cliff."),
		WorldSubsystem->CurrentDay,
		Resources.Gold,
		Resources.Timber,
		Resources.Stone,
		Resources.Supplies,
		ControlledCities,
		TotalCities,
		DailyGold,
		DailyTimber,
		DailyStone,
		DailySupplies,
		*CastlePhase,
		*WorldSubsystem->GetCastleAdvanceSummary(),
		*Ending,
		WorldSubsystem->Visitors.Num(),
		WorldSubsystem->PendingRumors.Num(),
		*WorldSubsystem->LastPrerumorText.ToString())));

	RefreshWorldVisuals();
}

void AIroncliffePrototypeDirector::RefreshWorldVisuals()
{
	if (!WorldSubsystem)
	{
		return;
	}

	const auto GetBuildingLevel = [this](EIroncliffeBuildingType Type)
	{
		const FIroncliffeBuildingState* Building = WorldSubsystem->Buildings.FindByPredicate([Type](const FIroncliffeBuildingState& Candidate)
		{
			return Candidate.Type == Type;
		});
		return Building ? Building->Level : 0;
	};

	const int32 LonghouseLevel = GetBuildingLevel(EIroncliffeBuildingType::Longhouse);
	if (LonghouseMesh)
	{
		LonghouseMesh->SetRelativeScale3D(FVector(1.45f + LonghouseLevel * 0.28f, 0.82f + LonghouseLevel * 0.10f, 0.60f + LonghouseLevel * 0.25f));
	}
	if (LonghouseRoof)
	{
		LonghouseRoof->SetRelativeLocation(FVector(-360.0f, -170.0f, 126.0f + LonghouseLevel * 18.0f));
		LonghouseRoof->SetRelativeScale3D(FVector(1.16f + LonghouseLevel * 0.18f, 0.58f + LonghouseLevel * 0.08f, 0.42f + LonghouseLevel * 0.08f));
	}
	if (LonghouseLabel)
	{
		LonghouseLabel->SetText(FText::FromString(FString::Printf(TEXT("Longhouse L%d\nE: upgrade"), LonghouseLevel)));
	}

	const int32 ForgeLevel = GetBuildingLevel(EIroncliffeBuildingType::Forge);
	if (ForgeMesh)
	{
		ForgeMesh->SetRelativeScale3D(FVector(0.85f + ForgeLevel * 0.16f, 0.62f + ForgeLevel * 0.08f, 0.58f + ForgeLevel * 0.14f));
	}

	const int32 MarketLevel = GetBuildingLevel(EIroncliffeBuildingType::Market);
	if (MarketMesh)
	{
		MarketMesh->SetRelativeScale3D(FVector(0.9f + MarketLevel * 0.16f, 0.70f + MarketLevel * 0.12f, 0.42f + MarketLevel * 0.12f));
	}

	const int32 CastlePhase = static_cast<int32>(WorldSubsystem->Castle.Phase);
	const auto SetPhaseVisible = [CastlePhase](UStaticMeshComponent* Component, int32 RequiredPhase)
	{
		if (Component)
		{
			Component->SetVisibility(CastlePhase >= RequiredPhase, true);
		}
	};

	SetPhaseVisible(CastleFoundation, 2);
	SetPhaseVisible(CastleWallNorth, 3);
	SetPhaseVisible(CastleWallSouth, 3);
	SetPhaseVisible(CastleWallWest, 3);
	SetPhaseVisible(CastleWallEast, 3);
	SetPhaseVisible(CastleGatehouse, 3);
	SetPhaseVisible(CastleInnerHall, 5);
	SetPhaseVisible(CastleKeep, 5);
	SetPhaseVisible(CastleTowerWest, 4);
	SetPhaseVisible(CastleTowerEast, 4);
	SetPhaseVisible(CastleTowerNorth, 4);
	SetPhaseVisible(CastleTowerSouth, 4);
	SetPhaseVisible(VaelFlagPole, 6);
	SetPhaseVisible(VaelFlag, 6);
	if (CastleLabel)
	{
		const FString PhaseName = StaticEnum<EIroncliffeCastlePhase>()->GetNameStringByValue(static_cast<int64>(WorldSubsystem->Castle.Phase));
		CastleLabel->SetText(FText::FromString(FString::Printf(TEXT("Vael's Cliff\nPhase: %s\n%s"), *PhaseName, *WorldSubsystem->GetCastleAdvanceSummary())));
	}

	for (const FIroncliffeLocationState& Location : WorldSubsystem->Locations)
	{
		const bool bLocationHidden = Location.bHiddenUntilDiscovered && !Location.bDiscovered;
		const FString Status = bLocationHidden
			? FString(TEXT("unknown"))
			: (Location.bControlled ? (Location.bTakenByForce ? FString(TEXT("conquered")) : FString(TEXT("allied"))) : FString(TEXT("open")));

		if (TObjectPtr<UTextRenderComponent>* LabelPtr = LocationLabels.Find(Location.Id))
		{
			if (UTextRenderComponent* Label = LabelPtr->Get())
			{
				if (Location.Type == EIroncliffeLocationType::City)
				{
					Label->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s | G%d D%d"), *Location.DisplayName.ToString(), *Status, Location.Garrison, Location.Diplomacy)));
				}
				else
				{
					Label->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s"), *Location.DisplayName.ToString(), *Status)));
				}
			}
		}

		if (TObjectPtr<UStaticMeshComponent>* MarkerPtr = LocationMarkers.Find(Location.Id))
		{
			if (UStaticMeshComponent* Marker = MarkerPtr->Get())
			{
				const FLinearColor ControlledColor = Location.bTakenByForce ? FLinearColor(0.62f, 0.12f, 0.10f, 1.0f) : FLinearColor(0.18f, 0.56f, 0.26f, 1.0f);
				const FLinearColor OpenColor = Location.Type == EIroncliffeLocationType::City ? FLinearColor(0.11f, 0.36f, 0.64f, 1.0f) : FLinearColor(0.62f, 0.59f, 0.48f, 1.0f);
				SetShapeColor(Marker, bLocationHidden ? FLinearColor(0.28f, 0.28f, 0.25f, 1.0f) : (Location.bControlled ? ControlledColor : OpenColor));

				if (Location.Type == EIroncliffeLocationType::City)
				{
					Marker->SetRelativeScale3D(Location.bControlled ? FVector(0.86f, 0.64f, 0.36f) : FVector(0.72f, 0.54f, 0.30f));
				}
				else if (Location.Type == EIroncliffeLocationType::Village)
				{
					Marker->SetRelativeScale3D(Location.bControlled ? FVector(0.34f, 0.34f, 0.26f) : FVector(0.26f, 0.26f, 0.20f));
				}
				else if (Location.Type == EIroncliffeLocationType::Hamlet)
				{
					Marker->SetRelativeScale3D(Location.bControlled ? FVector(0.23f, 0.23f, 0.18f) : FVector(0.16f, 0.16f, 0.13f));
				}
			}
		}
	}

	int32 WaitingVisitorCount = 0;
	for (const FIroncliffeRumorVisitor& Visitor : WorldSubsystem->Visitors)
	{
		if (Visitor.State == EIroncliffeCompanionState::WaitingAtGate)
		{
			WaitingVisitorCount++;
		}
	}

	for (int32 Index = 0; Index < VisitorMarkers.Num(); ++Index)
	{
		if (VisitorMarkers[Index])
		{
			VisitorMarkers[Index]->SetVisibility(Index < WaitingVisitorCount, true);
		}
	}
}

UStaticMeshComponent* AIroncliffePrototypeDirector::AddShape(FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color, UMaterialInterface* OverrideMaterial)
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this, Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(Mesh);
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(Rotation);
	Component->SetRelativeScale3D(Scale);
	Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (OverrideMaterial)
	{
		Component->SetMaterial(0, OverrideMaterial);
	}
	else if (ShapeMaterial)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ShapeMaterial, Component);
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Component->SetMaterial(0, DynamicMaterial);
	}
	Component->RegisterComponent();
	return Component;
}

void AIroncliffePrototypeDirector::AddInteractable(FName Name, const FVector& Location, EIroncliffeInteractionType InteractionType, const FString& Prompt, const FString& SuccessMessage, const FString& FailureMessage, FName LocationId, EIroncliffeBuildingType BuildingType)
{
	if (!GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = Name;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AIroncliffeInteractableActor* Interactable = GetWorld()->SpawnActor<AIroncliffeInteractableActor>(
		AIroncliffeInteractableActor::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (!Interactable)
	{
		return;
	}

	Interactable->Configure(
		InteractionType,
		FText::FromString(Prompt),
		FText::FromString(SuccessMessage),
		FText::FromString(FailureMessage),
		LocationId,
		BuildingType);
}

UTextRenderComponent* AIroncliffePrototypeDirector::AddLabel(FName Name, const FString& Text, const FVector& Location, float Size, const FColor& Color)
{
	UTextRenderComponent* Component = NewObject<UTextRenderComponent>(this, Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Component->SetHorizontalAlignment(EHTA_Left);
	Component->SetVerticalAlignment(EVRTA_TextCenter);
	Component->SetWorldSize(Size);
	Component->SetTextRenderColor(Color);
	Component->SetText(FText::FromString(Text));
	Component->RegisterComponent();
	return Component;
}
