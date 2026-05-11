// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IroncliffeGameMode.generated.h"

UCLASS()
class AIroncliffeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AIroncliffeGameMode();

protected:
	virtual void BeginPlay() override;
};




