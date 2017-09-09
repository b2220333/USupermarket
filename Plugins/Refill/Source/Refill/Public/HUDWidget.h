// Fill out your copyright notice in the Description page of Project Settings.

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refill")
		FString MyNewWidgetName;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refill")
		TMap<FString, UTexture2D*> ItemMap;

	UFUNCTION(BlueprintCallable, Category = "Refill")
		void OnUIEnabled();

	UFUNCTION(BlueprintCallable, Category = "Refill")
		void OnUIDisabled();

private:
	//ARAssetLoader* AssetLoader; // The AssetLoader actor
	ACacheAssetLoader* AssetLoader;

	TMap<UButton*, FString> ButtonToID;
	TArray<UListButton*> ButtonList;

	UFont* ButtonFont;


	//UFUNCTION()
		//FReply OnButtonClicked(FString f);

};
