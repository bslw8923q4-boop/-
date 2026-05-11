// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/IroncliffeTypes.h"
#include "IroncliffeInteractableActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EIroncliffeInteractionType : uint8
{
	RaidLocation UMETA(DisplayName = "Raid Location"),
	NegotiateLocation UMETA(DisplayName = "Negotiate Location"),
	UpgradeBuilding UMETA(DisplayName = "Upgrade Building"),
	AdvanceCastle UMETA(DisplayName = "Advance Castle"),
	AdvanceDay UMETA(DisplayName = "Advance Day"),
	AcceptFirstVisitor UMETA(DisplayName = "Accept First Visitor")
};

UCLASS()
class IRONCLIFFE_API AIroncliffeInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	AIroncliffeInteractableActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	EIroncliffeInteractionType InteractionType = EIroncliffeInteractionType::AdvanceDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	FName LocationId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	EIroncliffeBuildingType BuildingType = EIroncliffeBuildingType::Longhouse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	FText Prompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	FText SuccessMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	FText FailureMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Interaction")
	float InteractionRadius = 260.0f;

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Interaction")
	bool Interact(AController* InstigatorController, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Interaction")
	void Configure(EIroncliffeInteractionType NewInteractionType, FText NewPrompt, FText NewSuccessMessage, FText NewFailureMessage, FName NewLocationId = NAME_None, EIroncliffeBuildingType NewBuildingType = EIroncliffeBuildingType::Longhouse);

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Interaction")
	FString GetPromptString() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MarkerMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTextRenderComponent* PromptText;
};
