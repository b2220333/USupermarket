// Shows a list of available and placeable items in the player's HUD

#pragma once

#include "CoreMinimal.h"
#include "Buttons/ListButton.h"
#include "RAssetLoader.h"
#include "AssetLoader/CacheAssetLoader.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class REFILL_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UHUDWidget(const FObjectInitializer& PCIP);
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
	// Gets called when this widged gets enabled
	UFUNCTION(BlueprintCallable, Category = "Refill")
		void OnUIEnabled();

	// Gets called when this widged gets disabled
	UFUNCTION(BlueprintCallable, Category = "Refill")
		void OnUIDisabled();

private:
	ACacheAssetLoader* AssetLoader; // The AssetLoader instance

	UFont* ButtonFont; // The font used for the button texts 
};
