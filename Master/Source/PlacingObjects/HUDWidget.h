// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "Runtime/SlateCore/Public/SlateCore.h"
//#include "Runtime/SlateCore/Public/Input/Reply.h"
#include "Buttons/ListButton.h"
#include "Blueprint/UserWidget.h"
#include "AssetLoader/RAssetLoader.h"
#include "AssetLoader/CacheAssetLoader.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class PLACINGOBJECTS_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
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

	//UFUNCTION()
		//FReply OnButtonClicked(FString f);

};
