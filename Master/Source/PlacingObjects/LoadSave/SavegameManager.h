// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetLoader/CacheAssetLoader.h"
#include "RMyCharacter.h"
#include "Components/CPlaceItem.h"
#include "SavegameManager.generated.h"


UCLASS()
class PLACINGOBJECTS_API ASavegameManager : public AActor
{
	GENERATED_BODY()


public:
	// typedef TSharedRef < TJsonWriter< TCHAR, TPrettyJsonPrintPolicy = "" > > FPrettyJsonWriter;
	// typedef TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>> FPrettyJsonWriter;

	// Sets default values for this actor's properties
	ASavegameManager();

	UPROPERTY(EditAnywhere)
		bool bLoadLastSave;


	UPROPERTY(EditAnywhere)
		ARMyCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere)
		ACacheAssetLoader* AssetLoader;

	UCPlaceItem* PlaceComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SaveGame();
	void LoadGame();
	void PlaceItem(FString AssetPath, FVector Location, FRotator Rotation, TArray<FName> Tags);
};
