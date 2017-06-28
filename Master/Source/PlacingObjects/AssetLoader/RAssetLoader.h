// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Database/RIDatabase.h"

#include "Engine/TriggerBox.h"
#include "GameFramework/Actor.h"
#include "RAssetLoader.generated.h"

UCLASS()
class PLACINGOBJECTS_API ARAssetLoader : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "RAssetLoader")
		FString ItemToLoad;

	UPROPERTY(EditAnywhere, Category = "RAssetLoader")
		AActor* DatabaseInterface;

	UPROPERTY(EditAnywhere, Category = "RAssetLoader")
		ATriggerBox* SpawnPoint;

	TMap<FString, TPair<FString, FString>> IDToGUIParts;

	// Sets default values for this actor's properties
	ARAssetLoader();

private:
	AStaticMeshActor* SpawnedItem;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnAssetFromCache(const FString FileName, const FVector Location, const FRotator Rotation);
	void SpawnAsset(const FString Path, const FVector Location, const FRotator Rotation);
	void SpawnTest();
	
	AStaticMeshActor* GetSpawnedItem();
};
