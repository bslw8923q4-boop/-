// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/IroncliffeInteractableActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Systems/IroncliffeWorldSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AIroncliffeInteractableActor::AIroncliffeInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRadius);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(SceneRoot);
	MarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 48.0f));
	MarkerMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));
	MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(SceneRoot);
	PromptText->SetRelativeLocation(FVector(-95.0f, 0.0f, 128.0f));
	PromptText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	PromptText->SetHorizontalAlignment(EHTA_Left);
	PromptText->SetVerticalAlignment(EVRTA_TextCenter);
	PromptText->SetWorldSize(25.0f);
	PromptText->SetTextRenderColor(FColor(245, 232, 180));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereFinder.Succeeded())
	{
		MarkerMesh->SetStaticMesh(SphereFinder.Object);
	}

	Prompt = FText::FromString(TEXT("Press E"));
	SuccessMessage = FText::FromString(TEXT("Action completed."));
	FailureMessage = FText::FromString(TEXT("Action blocked."));
}

void AIroncliffeInteractableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InteractionSphere->SetSphereRadius(InteractionRadius);
	PromptText->SetText(FText::FromString(GetPromptString()));
}

bool AIroncliffeInteractableActor::Interact(AController* InstigatorController, FString& OutMessage)
{
	UIroncliffeWorldSubsystem* WorldState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;
	if (!WorldState)
	{
		OutMessage = TEXT("Ironcliffe world state is missing.");
		return false;
	}

	bool bSucceeded = false;
	switch (InteractionType)
	{
	case EIroncliffeInteractionType::RaidLocation:
		bSucceeded = WorldState->RaidLocation(LocationId);
		break;
	case EIroncliffeInteractionType::NegotiateLocation:
		bSucceeded = WorldState->NegotiateLocation(LocationId);
		break;
	case EIroncliffeInteractionType::UpgradeBuilding:
		bSucceeded = WorldState->UpgradeBuilding(BuildingType);
		break;
	case EIroncliffeInteractionType::AdvanceCastle:
		bSucceeded = WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic")));
		break;
	case EIroncliffeInteractionType::AdvanceDay:
		WorldState->AdvanceDay();
		bSucceeded = true;
		break;
	case EIroncliffeInteractionType::AcceptFirstVisitor:
		for (const FIroncliffeRumorVisitor& Visitor : WorldState->Visitors)
		{
			if (Visitor.State == EIroncliffeCompanionState::WaitingAtGate)
			{
				bSucceeded = WorldState->AcceptVisitor(Visitor.Id);
				break;
			}
		}
		break;
	default:
		break;
	}

	OutMessage = bSucceeded ? SuccessMessage.ToString() : FailureMessage.ToString();
	return bSucceeded;
}

void AIroncliffeInteractableActor::Configure(EIroncliffeInteractionType NewInteractionType, FText NewPrompt, FText NewSuccessMessage, FText NewFailureMessage, FName NewLocationId, EIroncliffeBuildingType NewBuildingType)
{
	InteractionType = NewInteractionType;
	Prompt = NewPrompt;
	SuccessMessage = NewSuccessMessage;
	FailureMessage = NewFailureMessage;
	LocationId = NewLocationId;
	BuildingType = NewBuildingType;
	PromptText->SetText(FText::FromString(GetPromptString()));
}

FString AIroncliffeInteractableActor::GetPromptString() const
{
	return FString::Printf(TEXT("E - %s"), *Prompt.ToString());
}
