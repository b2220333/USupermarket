// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "AssetLoader/RAssetLoader.h"
#include "ListButton.generated.h"

 //DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadDelegate, FString, ID);
 //DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClickDelegate);

/**
 *
 */
UCLASS()
class PLACINGOBJECTS_API UListButton : public UButton
{
	GENERATED_BODY()

private:
	UPROPERTY()
		FString AssetID;

	UPROPERTY()
	ARAssetLoader* AssetLoader;
public:
	UListButton();

	//UPROPERTY()
	//	FLoadDelegate load;

	//UPROPERTY()
	//	FClickDelegate click;

	UFUNCTION()
	void OnClick();

	//UFUNCTION()
	//void LoadAsset(FString ID);

	void SetupButton(ARAssetLoader* AssetLoader, FString AssetID);
};
