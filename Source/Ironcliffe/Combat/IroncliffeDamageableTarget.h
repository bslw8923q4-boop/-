// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IroncliffeDamageableTarget.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class IRONCLIFFE_API AIroncliffeDamageableTarget : public AActor
{
	GENERATED_BODY()

public:
	AIroncliffeDamageableTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Combat")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Combat")
	float MaxHealth = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	float Health = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Combat")
	int32 GoldReward = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Combat")
	int32 SuppliesReward = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ironcliffe|Combat")
	int32 RenownReward = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ironcliffe|Combat")
	bool bDefeated = false;

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Combat")
	bool ApplyIroncliffeDamage(float DamageAmount, AController* InstigatorController);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTextRenderComponent* HealthText;

	void RefreshVisuals();
	void GrantReward();
};
