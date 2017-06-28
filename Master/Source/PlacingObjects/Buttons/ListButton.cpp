// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "ListButton.h"


UListButton::UListButton()
{
	OnClicked.AddDynamic(this, &UListButton::OnClick);

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

//void UListButton::SetupButton(ARAssetLoader * AssetLoader, FString AssetID)
//{
//	this->AssetLoader = AssetLoader;
//	this->AssetID = AssetID;
//}
void UListButton::SetupButton(ACacheAssetLoader * AssetLoader, FString AssetID)
{
	this->AssetLoader = AssetLoader;
	this->AssetID = AssetID;
}


