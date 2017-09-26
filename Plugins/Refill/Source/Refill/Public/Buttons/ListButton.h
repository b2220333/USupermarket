// A button used for the inventory with functionality for OnHover and OnClick to spawn assets

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"

#include "../AssetLoader/RAssetLoader.h"
#include "../AssetLoader/CacheAssetLoader.h"
#include "ListButton.generated.h"

class AMyHUD; // Forward declaration

UCLASS()
class REFILL_API UListButton : public UButton
{
	GENERATED_BODY()

private:
	// The asset ID/name
	FString AssetID;

	// The AssetLoader instance
	ACacheAssetLoader* AssetLoader;

	// The HUD
	AMyHUD* MainHUD;
public:
	UListButton();

	// Gets called if the user clicks this button
	UFUNCTION()
		void OnClick();

	// Gets caled if the mouse hovers over this button
	UFUNCTION()
		void OnHover();

	// Gets called if the mouse no longer is hovering this button
	UFUNCTION()
		void OnUnhover();


	// Setup function for this button
	void SetupButton(ACacheAssetLoader* AssetLoader, FString AssetID);
};
