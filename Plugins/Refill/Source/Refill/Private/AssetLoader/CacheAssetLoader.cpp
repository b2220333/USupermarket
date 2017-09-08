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
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACacheAssetLoader::BeginPlay()
{
	Super::BeginPlay();

	IFileManager& FileManager = IFileManager::Get();
	CachePath = FPaths::GameContentDir().Append("Cache/Items/");
	FString ext = FString("*.uasset");
	FileManager.FindFiles(AssetsInChache, *CachePath, *ext);

	UE_LOG(LogTemp, Log, TEXT("%s: Loaded %i assets from cache"), *FString(__FUNCTION__), AssetsInChache.Num());
	for (auto&elem : AssetsInChache)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Loaded item %s"), *FString(__FUNCTION__), *elem);
	}
}

// Called every frame
void ACacheAssetLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACacheAssetLoader::SpawnAsset(const FString Path, const FVector Location, const FRotator Rotation)
{


	if (Path.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No path for spawning RefillObject."), *FString(__FUNCTION__));
		return;
	}
	UWorld* const World = GetWorld(); // get a reference to the world
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Cannot find the world."), *FString(__FUNCTION__));
		return;
	}

	if (CurrentObject != nullptr)
	{
		CurrentObject->Destroy();
		CurrentObject = nullptr;
	}


	FActorSpawnParameters SpawnInfo;

	//Create a new RefillObject in the world
	ARRefillObject* RefillObj = GetWorld()->SpawnActor<ARRefillObject>(Location, Rotation, SpawnInfo);

	//Path to the mesh for the RefillObject
	FString PathToAsset = FString(*CachePath);
	PathToAsset.Append(*Path);

	//	UE_LOG(LogTemp, Warning, TEXT("Spawning from path %s"), *PathToAsset);

	RefillObj->LoadRefillObject(PathToAsset);

	CurrentObject = RefillObj;
	CurrentObject->SetMobility(EComponentMobility::Movable);


	ReadAdditionalObjectParameters(RefillObj, PathToAsset);
	OnItemSpawend.Broadcast(RefillObj); // Tell everyone we spawned a new item
}

void ACacheAssetLoader::SpawnAsset(const FString Path)
{
	if (SpawnPoint == nullptr)
	{
		SpawnAsset(Path, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else
	{
		SpawnAsset(Path, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
	}
}

void ACacheAssetLoader::ReadAdditionalObjectParameters(ARRefillObject * RefillObj, FString PathToAsset)
{

	FString JsonPath = PathToAsset.Replace(*FString(".uasset"), *FString(".json"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*JsonPath)) {

		FString JsonObjectString;
		FFileHelper::LoadFileToString(JsonObjectString, *JsonPath);

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef< TJsonReader<> > ObjectReader = TJsonReaderFactory<>::Create(JsonObjectString);

		if (FJsonSerializer::Deserialize(ObjectReader, JsonObject))
		{
			for (auto& curItem : JsonObject->Values)
			{
				UE_LOG(LogTemp, Log, TEXT("%s: Loading json object %s"), *FString(__FUNCTION__), *curItem.Key);

				if (curItem.Key.Equals("Tag")) {
					RefillObj->Tags.Add(FName(*curItem.Value->AsString()));
				}

				// Check, if this item can be placed on a hook
				if (curItem.Key.Equals("HookPosition")) {
					auto& HookPositionJSONObj = *curItem.Value->AsObject();
					float HookX = HookPositionJSONObj.GetNumberField("X");
					float HookY = HookPositionJSONObj.GetNumberField("Y");
					float HookZ = HookPositionJSONObj.GetNumberField("Z");

					FVector HookPositionVector(HookX, HookY, HookZ);
					SetupHoleTab(RefillObj, HookPositionVector);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Deserialization failed"), *FString(__FUNCTION__));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("%s: File not found %s"), *FString(__FUNCTION__), *JsonPath);
	}

}

void ACacheAssetLoader::SetupHoleTab(ARRefillObject * RefillObj, FVector HoleTabPosition)
{
	UE_LOG(LogTemp, Log, TEXT("%s: Hook position %s"), *FString(__FUNCTION__), *HoleTabPosition.ToCompactString());
	UHoleTabComponent* HoleTab = NewObject<UHoleTabComponent>(RefillObj);
	HoleTab->AttachToComponent(RefillObj->GetStaticMeshComponent(), FAttachmentTransformRules::KeepWorldTransform);
	HoleTab->SetRelativeLocation(HoleTabPosition);
}

