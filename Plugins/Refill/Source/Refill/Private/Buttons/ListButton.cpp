// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "MyHUD.h"
#include "Engine.h"
#include "ListButton.h"


UListButton::UListButton()
{
	// Bind delegates
	OnClicked.AddDynamic(this, &UListButton::OnClick);
	OnHovered.AddDynamic(this, &UListButton::OnHover);
	OnUnhovered.AddDynamic(this, &UListButton::OnUnhover);
}

void UListButton::OnClick()
{
	if (AssetLoader != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UListButton::OnClick: Loading Asset %s"), *AssetID);
		AssetLoader->SpawnAsset(AssetID); // Spawn asset on click
	}
}

void UListButton::OnHover()
{
	if (MainHUD != nullptr && AssetLoader != nullptr) {
		// Get the image file name
		FString ImageFileName = AssetID.Replace(*FString(".uasset"), *FString(".png"));

		// Get path to the image file
		FString PathToImage = FPaths::GameContentDir().Append(AssetLoader->AssetPath).Append(ImageFileName);

		UE_LOG(LogTemp, Log, TEXT("%s: Loading image %s"), *FString(__FUNCTION__), *PathToImage);

		// Show image on HUD
		MainHUD->PreviewRefillItem(PathToImage, ImageFileName);
	}
}

void UListButton::OnUnhover()
{
	if (MainHUD != nullptr) {
		// Stop displaying the image of the item
		MainHUD->DisablePreviewImage();
	}
}

void UListButton::SetupButton(ACacheAssetLoader * _AssetLoader, FString _AssetID)
{
	this->AssetLoader = _AssetLoader;
	this->AssetID = _AssetID;

	// Assign the HUD
	if (AssetLoader != nullptr && AssetLoader->GetWorld() != nullptr && AssetLoader->GetWorld()->GetFirstPlayerController() != nullptr && AssetLoader->GetWorld()->GetFirstPlayerController()->GetHUD() != nullptr) {
		MainHUD = Cast<AMyHUD>(AssetLoader->GetWorld()->GetFirstPlayerController()->GetHUD());
	}
}


