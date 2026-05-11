// Copyright Epic Games, Inc. All Rights Reserved.

#include "IroncliffeGameMode.h"

#include "EngineUtils.h"
#include "IroncliffeCharacter.h"
#include "IroncliffePlayerController.h"
#include "Systems/IroncliffePrototypeDirector.h"
#include "UI/IroncliffeHUD.h"
#include "UObject/ConstructorHelpers.h"

AIroncliffeGameMode::AIroncliffeGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> ThirdPersonPawnClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (ThirdPersonPawnClass.Succeeded())
	{
		DefaultPawnClass = ThirdPersonPawnClass.Class;
	}
	else
	{
		DefaultPawnClass = AIroncliffeCharacter::StaticClass();
	}

	PlayerControllerClass = AIroncliffePlayerController::StaticClass();
	HUDClass = AIroncliffeHUD::StaticClass();
}

void AIroncliffeGameMode::BeginPlay()
{
	Super::BeginPlay();

	bool bHasPrototypeDirector = false;
	for (TActorIterator<AIroncliffePrototypeDirector> It(GetWorld()); It; ++It)
	{
		bHasPrototypeDirector = true;
		break;
	}

	if (!bHasPrototypeDirector && GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = TEXT("IroncliffePrototypeDirector");
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AIroncliffePrototypeDirector>(AIroncliffePrototypeDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	}
}
