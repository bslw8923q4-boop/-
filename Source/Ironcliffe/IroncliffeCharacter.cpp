// Copyright Epic Games, Inc. All Rights Reserved.

#include "IroncliffeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Combat/IroncliffeCombatComponent.h"
#include "Ironcliffe.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Systems/IroncliffeInteractableActor.h"
#include "Systems/IroncliffeWorldSubsystem.h"
#include "UI/IroncliffeHUD.h"
#include "UObject/ConstructorHelpers.h"

AIroncliffeCharacter::AIroncliffeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatComponent = CreateDefaultSubobject<UIroncliffeCombatComponent>(TEXT("IroncliffeCombat"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	PrototypeShapeMaterial = MaterialFinder.Object;

	PrototypeCloak = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SiriusPrototypeCloak"));
	PrototypeCloak->SetupAttachment(RootComponent);
	PrototypeCloak->SetStaticMesh(CubeFinder.Object);
	PrototypeCloak->SetRelativeLocation(FVector(-30.0f, 0.0f, 76.0f));
	PrototypeCloak->SetRelativeScale3D(FVector(0.18f, 0.72f, 1.28f));
	PrototypeCloak->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PrototypeChestPlate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SiriusPrototypeChestPlate"));
	PrototypeChestPlate->SetupAttachment(RootComponent);
	PrototypeChestPlate->SetStaticMesh(CubeFinder.Object);
	PrototypeChestPlate->SetRelativeLocation(FVector(32.0f, 0.0f, 92.0f));
	PrototypeChestPlate->SetRelativeScale3D(FVector(0.08f, 0.38f, 0.42f));
	PrototypeChestPlate->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PrototypeMedallion = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SiriusPrototypeMedallion"));
	PrototypeMedallion->SetupAttachment(RootComponent);
	PrototypeMedallion->SetStaticMesh(SphereFinder.Object);
	PrototypeMedallion->SetRelativeLocation(FVector(42.0f, 0.0f, 118.0f));
	PrototypeMedallion->SetRelativeScale3D(FVector(0.105f, 0.105f, 0.025f));
	PrototypeMedallion->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AIroncliffeCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplySiriusPrototypeLook();
	ShowPrototypeMessage(TEXT("Sirius Vael prototype active: M map, E/F interact, mouse/J/K/Q/Shift combat."));
}

void AIroncliffeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIroncliffeCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AIroncliffeCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIroncliffeCharacter::Look);

		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AIroncliffeCharacter::LightAttackInput);
		}

		if (HeavyAttackAction)
		{
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AIroncliffeCharacter::HeavyAttackInput);
		}

		if (ParryAction)
		{
			EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &AIroncliffeCharacter::ParryInput);
		}

		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AIroncliffeCharacter::DodgeInput);
		}
	}
	else
	{
		UE_LOG(LogIroncliffe, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AIroncliffeCharacter::LightAttackInput);
	PlayerInputComponent->BindKey(EKeys::J, IE_Pressed, this, &AIroncliffeCharacter::LightAttackInput);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AIroncliffeCharacter::HeavyAttackInput);
	PlayerInputComponent->BindKey(EKeys::K, IE_Pressed, this, &AIroncliffeCharacter::HeavyAttackInput);
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AIroncliffeCharacter::ParryInput);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AIroncliffeCharacter::DodgeInput);
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AIroncliffeCharacter::OrderAttackInput);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AIroncliffeCharacter::OrderHoldInput);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AIroncliffeCharacter::OrderFlankInput);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AIroncliffeCharacter::OrderRetreatInput);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AIroncliffeCharacter::RaidWestmirInput);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AIroncliffeCharacter::NegotiateWestmirInput);
	PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &AIroncliffeCharacter::UpgradeLonghouseInput);
	PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AIroncliffeCharacter::AdvanceCastleInput);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &AIroncliffeCharacter::AdvanceDayInput);
	PlayerInputComponent->BindKey(EKeys::M, IE_Pressed, this, &AIroncliffeCharacter::ToggleStrategicMapInput);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AIroncliffeCharacter::InteractInput);
	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AIroncliffeCharacter::InteractInput);
}

void AIroncliffeCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AIroncliffeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AIroncliffeCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AIroncliffeCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AIroncliffeCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AIroncliffeCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AIroncliffeCharacter::LightAttackInput()
{
	if (TryHandleStrategicMapClick())
	{
		return;
	}

	if (DoLightAttack())
	{
		DrawCombatCue(FColor(232, 218, 178), 135.0f, 42.0f);
		ShowPrototypeMessage(TEXT("Light attack: combo advanced."));
	}
	else
	{
		ShowPrototypeMessage(TEXT("Not enough stamina for light attack."));
	}
}

void AIroncliffeCharacter::HeavyAttackInput()
{
	if (DoHeavyAttack())
	{
		DrawCombatCue(FColor(210, 70, 55), 185.0f, 62.0f);
		ShowPrototypeMessage(TEXT("Heavy attack: block breaker committed."));
	}
	else
	{
		ShowPrototypeMessage(TEXT("Not enough stamina for heavy attack."));
	}
}

void AIroncliffeCharacter::ParryInput()
{
	if (DoParry())
	{
		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 78.0f), 94.0f, 18, FColor(96, 180, 255), false, 0.45f, 0, 3.0f);
		ShowPrototypeMessage(TEXT("Parry window open."));
	}
	else
	{
		ShowPrototypeMessage(TEXT("Not enough stamina to parry."));
	}
}

void AIroncliffeCharacter::DodgeInput()
{
	if (DoDodge())
	{
		const FVector DodgeVector = GetActorForwardVector() * 520.0f + FVector(0.0f, 0.0f, 80.0f);
		LaunchCharacter(DodgeVector, true, true);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 180.0f, 72.0f, FColor(120, 210, 180), false, 0.55f, 0, 4.0f);
		ShowPrototypeMessage(TEXT("Dodge spent stamina."));
	}
	else
	{
		ShowPrototypeMessage(TEXT("Not enough stamina to dodge."));
	}
}

void AIroncliffeCharacter::OrderAttackInput()
{
	SetTacticalOrder(EIroncliffeTacticalOrder::AttackTarget);
	ShowPrototypeMessage(TEXT("Order: attack target."));
}

void AIroncliffeCharacter::OrderHoldInput()
{
	SetTacticalOrder(EIroncliffeTacticalOrder::HoldPosition);
	ShowPrototypeMessage(TEXT("Order: hold position."));
}

void AIroncliffeCharacter::OrderFlankInput()
{
	SetTacticalOrder(EIroncliffeTacticalOrder::Flank);
	ShowPrototypeMessage(TEXT("Order: flank maneuver."));
}

void AIroncliffeCharacter::OrderRetreatInput()
{
	SetTacticalOrder(EIroncliffeTacticalOrder::Retreat);
	ShowPrototypeMessage(TEXT("Order: retreat. Morale takes the hit."));
}

void AIroncliffeCharacter::RaidWestmirInput()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		const FName LocationId = GetStrategicCommandLocationId();
		const FString LocationName = GetStrategicCommandLocationName();
		ShowPrototypeMessage(WorldState->RaidLocation(LocationId)
			? FString::Printf(TEXT("%s взят силой. Слухи уже пошли."), *LocationName)
			: FString::Printf(TEXT("Рейд на %s заблокирован."), *LocationName));
	}
}

void AIroncliffeCharacter::NegotiateWestmirInput()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		const FName LocationId = GetStrategicCommandLocationId();
		const FString LocationName = GetStrategicCommandLocationName();
		ShowPrototypeMessage(WorldState->NegotiateLocation(LocationId)
			? FString::Printf(TEXT("%s добровольно признаёт власть Ваэля."), *LocationName)
			: FString::Printf(TEXT("Переговоры с %s заблокированы."), *LocationName));
	}
}

void AIroncliffeCharacter::UpgradeLonghouseInput()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->UpgradeBuilding(EIroncliffeBuildingType::Longhouse) ? TEXT("Longhouse upgraded.") : TEXT("Longhouse upgrade blocked."));
	}
}

void AIroncliffeCharacter::AdvanceCastleInput()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		ShowPrototypeMessage(WorldState->AdvanceCastlePhase(FText::FromString(TEXT("Gothic"))) ? TEXT("Castle phase advanced.") : TEXT("Castle phase blocked."));
	}
}

void AIroncliffeCharacter::AdvanceDayInput()
{
	if (UIroncliffeWorldSubsystem* WorldState = GetIroncliffeWorldState())
	{
		WorldState->AdvanceDay();
		ShowPrototypeMessage(TEXT("A day passes. Rumors walk faster than soldiers."));
	}
}

void AIroncliffeCharacter::ToggleStrategicMapInput()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	AIroncliffeHUD* IroncliffeHUD = PlayerController ? Cast<AIroncliffeHUD>(PlayerController->GetHUD()) : nullptr;
	if (!IroncliffeHUD)
	{
		ShowPrototypeMessage(TEXT("Strategic map is not available yet."));
		return;
	}

	IroncliffeHUD->ToggleStrategicMap();
	if (PlayerController)
	{
		if (IroncliffeHUD->IsStrategicMapOpen())
		{
			PlayerController->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PlayerController->SetInputMode(InputMode);
		}
		else
		{
			PlayerController->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
		}
	}

	ShowPrototypeMessage(IroncliffeHUD->IsStrategicMapOpen() ? TEXT("Strategic map opened.") : TEXT("Strategic map closed."));
}

void AIroncliffeCharacter::InteractInput()
{
	if (!GetWorld())
	{
		return;
	}

	AIroncliffeInteractableActor* BestInteractable = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	const FVector PawnLocation = GetActorLocation();

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
		ShowPrototypeMessage(TEXT("No interaction nearby. Walk to a labeled marker and press E or F."));
		return;
	}

	FString Message;
	BestInteractable->Interact(GetController(), Message);
	ShowPrototypeMessage(Message);
}

UIroncliffeWorldSubsystem* AIroncliffeCharacter::GetIroncliffeWorldState() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UIroncliffeWorldSubsystem>() : nullptr;
}

bool AIroncliffeCharacter::TryHandleStrategicMapClick()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	AIroncliffeHUD* IroncliffeHUD = PlayerController ? Cast<AIroncliffeHUD>(PlayerController->GetHUD()) : nullptr;
	if (!IroncliffeHUD || !IroncliffeHUD->IsStrategicMapOpen())
	{
		return false;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		ShowPrototypeMessage(TEXT("Наведи курсор на карточку города и кликни."));
		return true;
	}

	FString Message;
	IroncliffeHUD->SelectStrategicMapLocation(FVector2D(MouseX, MouseY), Message);
	ShowPrototypeMessage(Message);
	return true;
}

FName AIroncliffeCharacter::GetStrategicCommandLocationId() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const AIroncliffeHUD* IroncliffeHUD = PlayerController ? Cast<AIroncliffeHUD>(PlayerController->GetHUD()) : nullptr;
	return IroncliffeHUD && IroncliffeHUD->IsStrategicMapOpen() ? IroncliffeHUD->GetSelectedStrategicLocationId() : FName(TEXT("westmir"));
}

FString AIroncliffeCharacter::GetStrategicCommandLocationName() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const AIroncliffeHUD* IroncliffeHUD = PlayerController ? Cast<AIroncliffeHUD>(PlayerController->GetHUD()) : nullptr;
	return IroncliffeHUD && IroncliffeHUD->IsStrategicMapOpen() ? IroncliffeHUD->GetSelectedStrategicLocationName() : FString(TEXT("Вестмир"));
}

void AIroncliffeCharacter::ShowPrototypeMessage(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(9001, 1.65f, FColor(232, 218, 178), Message);
	}
}

void AIroncliffeCharacter::DrawCombatCue(const FColor& Color, float Reach, float Size) const
{
	if (!GetWorld())
	{
		return;
	}

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 74.0f);
	const FVector End = Start + GetActorForwardVector() * Reach;
	DrawDebugDirectionalArrow(GetWorld(), Start, End, Size, Color, false, 0.42f, 0, 5.0f);
	DrawDebugSphere(GetWorld(), End, Size * 0.42f, 12, Color, false, 0.42f, 0, 3.0f);
}

void AIroncliffeCharacter::ApplySiriusPrototypeLook()
{
	ApplyPrototypeShapeColor(PrototypeCloak, FLinearColor(0.015f, 0.014f, 0.013f, 1.0f));
	ApplyPrototypeShapeColor(PrototypeChestPlate, FLinearColor(0.58f, 0.60f, 0.58f, 1.0f));
	ApplyPrototypeShapeColor(PrototypeMedallion, FLinearColor(0.77f, 0.73f, 0.62f, 1.0f));

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		const int32 MaterialCount = CharacterMesh->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateAndSetMaterialInstanceDynamic(Index);
			if (!DynamicMaterial)
			{
				continue;
			}

			const FLinearColor Tint = Index == 0
				? FLinearColor(0.10f, 0.105f, 0.11f, 1.0f)
				: FLinearColor(0.42f, 0.43f, 0.42f, 1.0f);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		}
	}
}

void AIroncliffeCharacter::ApplyPrototypeShapeColor(UStaticMeshComponent* Component, const FLinearColor& Color) const
{
	if (!Component || !PrototypeShapeMaterial)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PrototypeShapeMaterial, Component);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Component->SetMaterial(0, DynamicMaterial);
}

bool AIroncliffeCharacter::DoLightAttack()
{
	return CombatComponent ? CombatComponent->LightAttack() : false;
}

bool AIroncliffeCharacter::DoHeavyAttack()
{
	return CombatComponent ? CombatComponent->HeavyAttack() : false;
}

bool AIroncliffeCharacter::DoParry()
{
	return CombatComponent ? CombatComponent->Parry() : false;
}

bool AIroncliffeCharacter::DoDodge()
{
	return CombatComponent ? CombatComponent->Dodge() : false;
}

void AIroncliffeCharacter::SetTacticalOrder(EIroncliffeTacticalOrder NewOrder)
{
	if (CombatComponent)
	{
		CombatComponent->SetTacticalOrder(NewOrder);
	}
}

