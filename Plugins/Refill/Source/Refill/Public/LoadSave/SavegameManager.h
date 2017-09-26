// Handles loading and saving of the current game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../AssetLoader/CacheAssetLoader.h"
#include "ItemManager.h"
#include "RMyCharacter.h"
#include "../Components/CPlaceItem.h"
#include "SavegameManager.generated.h"


UCLASS()
class REFILL_API ASavegameManager : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ASavegameManager();

	// The AssetLoader instance
	UPROPERTY(EditAnywhere, Category = "Refills")
		ACacheAssetLoader* AssetLoader;

	// The Name of the savegame file
	UPROPERTY(EditAnywhere, Category = "Refills")
		FString SavegameFileName;

	// The folder of the savegame file
	UPROPERTY(EditAnywhere, Category = "Refills")
		FString SaveDirectory;

	UCPlaceItem* PlaceComponent; // The player's CPlaceItem Component

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Save the game
	void SaveGame();

	// Load the game
	void LoadGame();

	// Place an item in the world
	ARRefillObject* PlaceItem(FString AssetName, FVector Location, FRotator Rotation, TArray<FName> Tags);

private:
	ARMyCharacter* PlayerCharacter; // Character instance
	AItemManager* ItemManager; // ItemManager instance
};
