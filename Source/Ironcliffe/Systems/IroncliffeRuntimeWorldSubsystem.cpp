// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/IroncliffeRuntimeWorldSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Ironcliffe.h"
#include "IroncliffeCharacter.h"
#include "Systems/IroncliffePrototypeDirector.h"
#include "TimerManager.h"
#include "UI/IroncliffeHUD.h"

bool UIroncliffeRuntimeWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UIroncliffeRuntimeWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!InWorld.IsGameWorld())
	{
		return;
	}

	InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		BootstrapWorld();
	}));
}

void UIroncliffeRuntimeWorldSubsystem::BootstrapWorld()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	bool bHadDirector = false;
	for (TActorIterator<AIroncliffePrototypeDirector> It(World); It; ++It)
	{
		bHadDirector = true;
		break;
	}

	if (!bHadDirector)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("Ironcliffe_Runtime_PrototypeDirector");
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AIroncliffePrototypeDirector>(AIroncliffePrototypeDirector::StaticClass(), FTransform::Identity, SpawnParams);
	}

	int32 ControllerCount = 0;
	bool bHasIroncliffePawn = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		++ControllerCount;
		bHasIroncliffePawn = bHasIroncliffePawn || Cast<AIroncliffeCharacter>(PlayerController->GetPawn()) != nullptr;

		if (!Cast<AIroncliffeHUD>(PlayerController->GetHUD()))
		{
			PlayerController->ClientSetHUD(AIroncliffeHUD::StaticClass());
		}
	}

	const FString Status = FString::Printf(
		TEXT("IRONCLIFFE BOOTSTRAP ACTIVE | Director: %s | Controllers: %d | Sirius pawn: %s"),
		bHadDirector ? TEXT("already running") : TEXT("spawned"),
		ControllerCount,
		bHasIroncliffePawn ? TEXT("yes") : TEXT("template pawn"));

	UE_LOG(LogIroncliffe, Warning, TEXT("%s"), *Status);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7001, 8.0f, FColor::Green, Status);
	}
}
