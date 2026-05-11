// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "IroncliffeHUD.generated.h"

UCLASS()
class IRONCLIFFE_API AIroncliffeHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Map")
	void ToggleStrategicMap();

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Map")
	bool IsStrategicMapOpen() const { return bStrategicMapOpen; }

	UFUNCTION(BlueprintCallable, Category = "Ironcliffe|Map")
	bool SelectStrategicMapLocation(const FVector2D& ScreenPosition, FString& OutMessage);

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Map")
	FName GetSelectedStrategicLocationId() const { return SelectedStrategicLocationId; }

	UFUNCTION(BlueprintPure, Category = "Ironcliffe|Map")
	FString GetSelectedStrategicLocationName() const { return SelectedStrategicLocationName; }

private:
	bool bStrategicMapOpen = false;
	FName SelectedStrategicLocationId = TEXT("westmir");
	FString SelectedStrategicLocationName = TEXT("Westmir");

	void DrawCompactHud();
	void DrawStrategicMap();
	void DrawStrategicCard(const FString& Title, const FString& Descriptor, const FString& Direction, float X, float Y, float Width, float Height, const FLinearColor& Accent, bool bControlled, bool bTakenByForce, bool bSelected = false);
	void DrawMapNode(const FString& Title, const FString& Subtitle, float X, float Y, float Radius, const FLinearColor& Accent);
	void DrawLegendItem(const FString& Label, float X, float Y, const FLinearColor& Color);
	void DrawMapButton(const FString& Label, float X, float Y, float Width, float Height, bool bPrimary);
	void DrawCircleOutline(float CenterX, float CenterY, float Radius, const FLinearColor& Color, float Thickness = 2.0f, int32 Segments = 28);
	void DrawFrame(float X, float Y, float Width, float Height, const FLinearColor& Color, float Thickness = 2.0f);
};
