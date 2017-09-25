// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "FileManagerGeneric.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "../Components/HoleTabComponent.h"
#include "TagStatics.h"
#include "CacheAssetLoader.h"



// Sets default values
ACacheAssetLoader::ACacheAssetLoader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	AssetPath = FString("Cache/Items/");
}

// Called when the game starts or when spawned
void ACacheAssetLoader::BeginPlay()
{
	Super::BeginPlay();

	// Load all files in cache folder on start
	ReloadAssetsFromCache();
}

// Called every frame
void ACacheAssetLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ARRefillObject* ACacheAssetLoader::SpawnAsset(const FString _AssetName, const FVector Location, const FRotator Rotation)
{
	if (_AssetName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No path for spawning RefillObject."), *FString(__FUNCTION__));
		return nullptr;
	}

	FString AssetName = _AssetName;
	if (AssetName.Contains(".uasset") == false)	 AssetName = _AssetName + ".uasset";

	if (CurrentObject != nullptr)
	{
		// We destroy the current existing object before spawning the next one
		CurrentObject->Destroy();
		CurrentObject = nullptr;
	}

	FActorSpawnParameters SpawnInfo;

	//Create a new RefillObject in the world
	CurrentObject = GetWorld()->SpawnActor<ARRefillObject>(Location, Rotation, SpawnInfo);

	//Path to the mesh for the RefillObject
	FString CachePath = FPaths::GameContentDir().Append(AssetPath);
	FString PathToAsset = FString(*CachePath);
	PathToAsset.Append(*AssetName);

	CurrentObject->LoadRefillObject(PathToAsset);

	if (RefillObjectInfo.Contains(AssetName)) {
		CurrentObject->ObjectInfo = RefillObjectInfo[AssetName]; // Add the objects info to itself
		CurrentObject->Tags = CurrentObject->ObjectInfo.Tags; // Add the tags to the actor
		CurrentObject->Tags.Add("Refill;RefillObject,True;"); // TODO Hardcoded string

		// Check if this item can be placed on a hook
		if (CurrentObject->ObjectInfo.bCanBeHookedUp) {
			SetupHoleTab(CurrentObject, CurrentObject->ObjectInfo.HolePosition);
		}
	}

	CurrentObject->SetMobility(EComponentMobility::Movable);

	OnItemSpawend.Broadcast(CurrentObject); // Tell everyone who wants to know that we spawned a new item

	return CurrentObject;
}

ARRefillObject* ACacheAssetLoader::SpawnAsset(const FString AssetName)
{
	if (SpawnPoint == nullptr)
	{
		return SpawnAsset(AssetName, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else
	{
		return SpawnAsset(AssetName, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
	}
}

int ACacheAssetLoader::ReloadAssetsFromCache()
{
	IFileManager& FileManager = IFileManager::Get();
	FString CachePath = FPaths::GameContentDir().Append(AssetPath);
	FString ext = FString("*.uasset");
	FileManager.FindFiles(AssetsInChache, *CachePath, *ext);

	UE_LOG(LogTemp, Log, TEXT("%s: Loaded %i assets from cache"), *FString(__FUNCTION__), AssetsInChache.Num());
	for (auto&elem : AssetsInChache)
	{
		ReadAdditionalObjectParameters(elem, FPaths::GameContentDir().Append(AssetPath).Append(elem));
		UE_LOG(LogTemp, Log, TEXT("%s: Loaded item %s"), *FString(__FUNCTION__), *elem);
	}
	return AssetsInChache.Num();
}

void ACacheAssetLoader::ReadAdditionalObjectParameters(FString AssetName, FString PathToAsset)
{
	FString JsonPath = PathToAsset;

	JsonPath.RemoveFromEnd(".uasset"); // Remove, if this path has this suffix
	JsonPath = JsonPath.Append(".json");

	FRefillObjectInfo ObjInfo;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*JsonPath)) {
		FString JsonObjectString;
		FFileHelper::LoadFileToString(JsonObjectString, *JsonPath);

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> ObjectReader = TJsonReaderFactory<>::Create(JsonObjectString);

		if (FJsonSerializer::Deserialize(ObjectReader, JsonObject))
		{
			bool bIsHookableItem = false;

			for (auto& curItem : JsonObject->Values)
			{
				UE_LOG(LogTemp, Log, TEXT("%s: Loading json object %s"), *FString(__FUNCTION__), *curItem.Key);

				// *** Read TAGs *** 
				if (curItem.Key.Equals("Tags")) { // TODO hardcoded string
					TArray<FName> RefillItemTags;

					for (auto& singleTag : curItem.Value->AsArray()) {
						RefillItemTags.Add(FName(*singleTag->AsString()));
					}

					ObjInfo.Tags = RefillItemTags;
					continue;
				}

				// *** Check, if this item can be placed on a hook ***
				if (curItem.Key.Equals("HookPosition")) { // TODO hardcoded string
					auto& HookPositionJSONObj = *curItem.Value->AsObject();
					float HookX = HookPositionJSONObj.GetNumberField("X");
					float HookY = HookPositionJSONObj.GetNumberField("Y");
					float HookZ = HookPositionJSONObj.GetNumberField("Z");

					FVector HookPositionVector(HookX, HookY, HookZ);
					bIsHookableItem = true;
					ObjInfo.HolePosition = HookPositionVector;
					continue;
				}

				// *** Read name ***
				if (curItem.Key.Equals("Name")) { // TODO hardcoded string
					ObjInfo.Name = curItem.Value->AsString();
					continue;
				}

				// Read company ***
				if (curItem.Key.Equals("Company")) { // TODO hardcoded string
					ObjInfo.Company = curItem.Value->AsString();
					continue;
				}
			}

			ObjInfo.bCanBeHookedUp = bIsHookableItem;
			RefillObjectInfo.Add(AssetName, ObjInfo); // Add it to the map
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Deserialization failed"), *FString(__FUNCTION__));
		}
	}
	else {
		// We couldn't find the json. We use only the asset name 
		ObjInfo.Name = AssetName;

		// If it hasn't a json file it can't be placed on a hook
		ObjInfo.bCanBeHookedUp = false;

		RefillObjectInfo.Add(AssetName, ObjInfo);

		UE_LOG(LogTemp, Warning, TEXT("%s: File not found %s"), *FString(__FUNCTION__), *JsonPath);
	}
}

void ACacheAssetLoader::SetupHoleTab(ARRefillObject * RefillObj, FVector HoleTabPosition)
{
	UE_LOG(LogTemp, Log, TEXT("%s: Hook position %s"), *FString(__FUNCTION__), *HoleTabPosition.ToCompactString());

	UHoleTabComponent* HoleTab = NewObject<UHoleTabComponent>(RefillObj);
	HoleTab->SetupAttachment(RefillObj->GetStaticMeshComponent());
	HoleTab->RegisterComponent();
	HoleTab->SetRelativeLocation(HoleTabPosition);
}

