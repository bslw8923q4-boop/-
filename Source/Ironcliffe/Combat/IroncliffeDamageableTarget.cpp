// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/IroncliffeDamageableTarget.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Systems/IroncliffeWorldSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AIroncliffeDamageableTarget::AIroncliffeDamageableTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(SceneRoot);
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
	BodyMesh->SetRelativeScale3D(FVector(0.58f, 0.58f, 1.45f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionObjectType(ECC_WorldDynamic);

	HealthText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HealthText"));
	HealthText->SetupAttachment(SceneRoot);
	HealthText->SetRelativeLocation(FVector(-96.0f, 0.0f, 190.0f));
	HealthText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	HealthText->SetHorizontalAlignment(EHTA_Left);
	HealthText->SetVerticalAlignment(EVRTA_TextCenter);
	HealthText->SetWorldSize(22.0f);
	HealthText->SetTextRenderColor(FColor(232, 218, 178));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
	}

	DisplayName = FText::FromString(TEXT("Raid Target"));
}

void AIroncliffeDamageableTarget::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	RefreshVisuals();
}

void AIroncliffeDamageableTarget::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Health = FMath::Clamp(Health <= 0.0f ? MaxHealth : Health, 0.0f, MaxHealth);
	RefreshVisuals();
}

bool AIroncliffeDamageableTarget::ApplyIroncliffeDamage(float DamageAmount, AController* InstigatorController)
{
	if (bDefeated || DamageAmount <= 0.0f)
	{
		return false;
	}

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	if (Health <= 0.0f)
	{
		bDefeated = true;
		GrantReward();
	}

	RefreshVisuals();
	return true;
}

void AIroncliffeDamageableTarget::RefreshVisuals()
{
	const float HealthRatio = MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;

	if (HealthText)
	{
		const FString StateText = bDefeated
			? FString::Printf(TEXT("%s\ndefeated"), *DisplayName.ToString())
			: FString::Printf(TEXT("%s\nHP %.0f/%.0f"), *DisplayName.ToString(), Health, MaxHealth);
		HealthText->SetText(FText::FromString(StateText));
	}

	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(bDefeated ? FVector(0.72f, 0.72f, 0.18f) : FVector(0.58f + (1.0f - HealthRatio) * 0.18f, 0.58f, 0.65f + HealthRatio * 0.8f));
		BodyMesh->SetRelativeRotation(bDefeated ? FRotator(80.0f, 0.0f, 0.0f) : FRotator::ZeroRotator);

		if (UMaterialInstanceDynamic* DynamicMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			const FLinearColor Color = bDefeated
				? FLinearColor(0.12f, 0.11f, 0.10f, 1.0f)
				: FLinearColor(0.50f + (1.0f - HealthRatio) * 0.30f, 0.10f + HealthRatio * 0.18f, 0.08f, 1.0f);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
	}
}

void AIroncliffeDamageableTarget::GrantReward()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UIroncliffeWorldSubsystem* WorldState = GameInstance->GetSubsystem<UIroncliffeWorldSubsystem>())
		{
			WorldState->Resources.Gold += GoldReward;
			WorldState->Resources.Supplies += SuppliesReward;
			WorldState->Resources.Renown += RenownReward;
			WorldState->AddRumorWeight(EIroncliffeRumorTrigger::SiegeWon, 12);
			WorldState->OnWorldChanged.Broadcast();
		}
	}
}
