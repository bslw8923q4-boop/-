// Copyright Epic Games, Inc. All Rights Reserved.


#include "IroncliffePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "Ironcliffe.h"
#include "IroncliffeCharacter.h"
#include "Systems/IroncliffeInteractableActor.h"
#include "Systems/IroncliffeWorldSubsystem.h"
#include "UI/IroncliffeHUD.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AIroncliffePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ClientSetHUD(AIroncliffeHUD::StaticClass());
		ShowPrototypeMessage(TEXT("Ironcliffe prototype loaded: R raid, N negotiate, B build, C castle, T day."));
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogIroncliffe, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AIroncliffePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AIroncliffePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AIroncliffePlayerController::HandleLightAttack()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		ShowPrototypeMessage(IroncliffePawn->DoLightAttack() ? TEXT("Light attack: Sirius keeps the combo tight.") : TEXT("Not enough stamina for light attack."));
	}
}

void AIroncliffePlayerController::HandleHeavyAttack()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		ShowPrototypeMessage(IroncliffePawn->DoHeavyAttack() ? TEXT("Heavy attack: block breaker committed.") : TEXT("Not enough stamina for heavy attack."));
	}
}

void AIroncliffePlayerController::HandleParry()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		ShowPrototypeMessage(IroncliffePawn->DoParry() ? TEXT("Parry window open. Tiny, cruel, beautiful.") : TEXT("Not enough stamina to parry."));
	}
}

void AIroncliffePlayerController::HandleDodge()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		ShowPrototypeMessage(IroncliffePawn->DoDodge() ? TEXT("Dodge spent stamina.") : TEXT("Not enough stamina to dodge."));
	}
}

void AIroncliffePlayerController::HandleOrderAttack()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		IroncliffePawn->SetTacticalOrder(EIroncliffeTacticalOrder::AttackTarget);
		ShowPrototypeMessage(TEXT("Order: attack target."));
	}
}

void AIroncliffePlayerController::HandleOrderHold()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		IroncliffePawn->SetTacticalOrder(EIroncliffeTacticalOrder::HoldPosition);
		ShowPrototypeMessage(TEXT("Order: hold position."));
	}
}

void AIroncliffePlayerController::HandleOrderFlank()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		IroncliffePawn->SetTacticalOrder(EIroncliffeTacticalOrder::Flank);
		ShowPrototypeMessage(TEXT("Order: flank maneuver."));
	}
}

void AIroncliffePlayerController::HandleOrderRetreat()
{
	if (AIroncliffeCharacter* IroncliffePawn = Cast<AIroncliffeCharacter>(GetPawn()))
	{
		IroncliffePawn->SetTacticalOrder(EIroncliffeTacticalOrder::Retreat);
		ShowPrototypeMessage(TEXT("Order: retreat. Morale takes the hit."));
	}
}

void AIroncliffePlayerController::HandleRaidWestmir()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->RaidLocation(TEXT("westmir")) ? TEXT("Westmir taken by force. Rumors start moving.") : TEXT("Westmir raid failed or already resolved."));
	}
}

void AIroncliffePlayerController::HandleNegotiateWestmir()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->NegotiateLocation(TEXT("westmir")) ? TEXT("Westmir accepts Vael's rule voluntarily.") : TEXT("Negotiation failed or Westmir already resolved."));
	}
}

void AIroncliffePlayerController::HandleUpgradeLonghouse()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->UpgradeBuilding(EIroncliffeBuildingType::Longhouse) ? TEXT("Longhouse upgraded. The settlement looks less like a camp.") : TEXT("Longhouse upgrade blocked: resources or max level."));
	}
}

void AIroncliffePlayerController::HandleAdvanceCastle()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))) ? TEXT("Castle phase advanced on Vael's cliff.") : TEXT("Castle phase blocked: take a location or gather resources."));
	}
}

void AIroncliffePlayerController::HandleAdvanceDay()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		WorldState->AdvanceDay();
		ShowPrototypeMessage(TEXT("A day passes. Rumors walk faster than soldiers."));
	}
}

void AIroncliffePlayerController::HandleInteract()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return;
	}

	AIroncliffeInteractableActor* BestInteractable = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	const FVector PawnLocation = ControlledPawn->GetActorLocation();

	for (TActorIterator<AIroncliffeInteractableActor> It(GetWorld()); It; ++It)
	{
		AIroncliffeInteractableActor* Interactable = *It;
		const float DistanceSq = FVector::DistSquared(PawnLocation, Interactable->GetActorLocation());
		const float RadiusSq = FMath::Square(Interactable->InteractionRadius);
		if (DistanceSq <= RadiusSq && DistanceSq < BestDistanceSq)
		{
			BestInteractable = Interactable;
			BestDistanceSq = DistanceSq;
		}
	}

	if (!BestInteractable)
	{
		ShowPrototypeMessage(TEXT("No Ironcliffe interaction nearby. Walk to a labeled marker and press E."));
		return;
	}

	FString Message;
	BestInteractable->Interact(this, Message);
	ShowPrototypeMessage(Message);
}

UIroncliffeWorldSubsystem* AIroncliffePlayerController::GetIroncliffeWorldState() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;
}

void AIroncliffePlayerController::ShowPrototypeMessage(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(9002, 1.65f, FColor(232, 218, 178), Message);
	}
}

