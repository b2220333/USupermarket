// Loads assets (.uasset, .json, .png) from the cache folder

#pragma once

#include "CoreMinimal.h"
#include "RRefillObject.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Actor.h"
#include "RefillObjectInfo.h"
#include "CacheAssetLoader.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemSpawned, AActor*, SpawnedActor);

UCLASS()
class REFILL_API ACacheAssetLoader : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACacheAssetLoader();

	// The spawn point where new items gets spawned. Should somewhere hidden (like far far away)
	UPROPERTY(EditAnywhere, Category = "Refills")
		ATriggerBox* SpawnPoint;

	// The relative location of the items folder within the content folder 
	UPROPERTY(EditAnywhere, Category = "Refills")
		FString AssetPath;

	// The names of all assets found in the cache folder
	TArray<FString> AssetsInChache;

	// Maps the asset name to their respective info structure
	TMap<FString, FRefillObjectInfo> RefillObjectInfo;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Spawns an asset with the given name at the given location
	ARRefillObject* SpawnAsset(const FString AssetName, const FVector Location, const FRotator Rotation);

	// Spawns an asset with the given name at the loacation of the spawn point if existing
	ARRefillObject* SpawnAsset(const FString AssetName);

	// The delegate which gets broadcasted if a new item has been spawned
	UPROPERTY()
		FItemSpawned OnItemSpawend;

	// The current spawned item
	ARRefillObject* CurrentObject;

	// Reloads all assets from cache
	int ReloadAssetsFromCache();

private:
	// Reads additional information provided by json files within the asset path folder
	void ReadAdditionalObjectParameters(FString AssetName, FString PathToAsset);

	// Sets up the hole component for items which can be placed on a hook
	void SetupHoleTab(ARRefillObject* RefillObj, FVector HoleTabPosition);
};
