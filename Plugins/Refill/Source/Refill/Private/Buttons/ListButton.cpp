// Fill out your copyright notice in the Description page of Project Settings.
#define IMAGE_FOLDER "Cache/Items/" // The path in the content folder to the images of the items

#include "Refill.h"
#include "MyHUD.h"
#include "Engine.h"
#include "ListButton.h"


UListButton::UListButton()
{
	OnClicked.AddDynamic(this, &UListButton::OnClick);
	OnHovered.AddDynamic(this, &UListButton::OnHover);
	OnUnhovered.AddDynamic(this, &UListButton::OnUnhover);


	//Bind function
	//load.AddDynamic(this, &UListButton::LoadAsset);
}

void UListButton::OnClick()
{


	if (AssetLoader != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UListButton::OnClick: Loading Asset %s"), *AssetID);
		// TODO AssetLoader->LoadAsset(AssetID);
		AssetLoader->SpawnAsset(AssetID);
	}
}

void UListButton::OnHover()
{
	if (MainHUD != nullptr) {
		FString ImageFileName = AssetID.Replace(*FString(".uasset"), *FString(".png"));
		FString PathToImage = FPaths::GameContentDir().Append(IMAGE_FOLDER).Append(ImageFileName);

		UE_LOG(LogTemp, Log, TEXT("%s: Loading image %s"), *FString(__FUNCTION__), *PathToImage);
		MainHUD->PreviewRefillItem(PathToImage, ImageFileName);
	}
}

void UListButton::OnUnhover()
{
	if (MainHUD != nullptr) {		
		MainHUD->DisablePreviewImage();
	}
}

//void UListButton::SetupButton(ARAssetLoader * AssetLoader, FString AssetID)
//{
//	this->AssetLoader = AssetLoader;
//	this->AssetID = AssetID;
//}
void UListButton::SetupButton(ACacheAssetLoader * _AssetLoader, FString _AssetID)
{
	this->AssetLoader = _AssetLoader;
	this->AssetID = _AssetID;

	// Assign the HUD
	if (AssetLoader != nullptr) MainHUD = Cast<AMyHUD>(AssetLoader->GetWorld()->GetFirstPlayerController()->GetHUD());
}


