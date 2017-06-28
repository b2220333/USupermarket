// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "FileManagerGeneric.h"
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

	UE_LOG(LogTemp, Warning, TEXT("Loaded %i assets from cache"), AssetsInChache.Num());
	for (auto&elem : AssetsInChache)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *elem);
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
		UE_LOG(LogTemp, Warning, TEXT("No path for spawning RefillObject."));
		return;
	}
	UWorld* const World = GetWorld(); // get a reference to the world
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find the world."));
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

	RefillObj->LoadRefillObject(PathToAsset);

	CurrentObject = RefillObj;
	CurrentObject->SetMobility(EComponentMobility::Movable);
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

void ACacheAssetLoader::SelectAsset(FString AssetName)
{
}

