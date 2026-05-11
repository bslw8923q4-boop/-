// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "IroncliffeRuntimeWorldSubsystem.generated.h"

/**
 * Runtime bootstrap that keeps the Ironcliffe prototype active even when an
 * editor map still has the stock ThirdPerson GameMode override.
 */
UCLASS()
class IRONCLIFFE_API UIroncliffeRuntimeWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void BootstrapWorld();
};
