// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"

#include "FileManagerGeneric.h"
#include "RRefillObject.h"
#include "RAssetLoader.h"

// Sets default values
ARAssetLoader::ARAssetLoader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ItemToLoad = "Mug";
}

// Called when the game starts or when spawned
void ARAssetLoader::BeginPlay()
{
	Super::BeginPlay();

	if (Cast<IRIDatabase>(DatabaseInterface) == nullptr)
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "The Database Interface is not set.");

	//This will take care of binding the method SpawnAllAssets to the button 'e'
	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()->GetPawn()->GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
		InputComponent->BindAction("SpawnRObject", IE_Pressed, this, &ARAssetLoader::SpawnTest);
	}

	IRIDatabase* DatabaseHandler = Cast<IRIDatabase>(DatabaseInterface);

	if (DatabaseHandler == nullptr)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red,
			"The Database Interface is not set, so the object cannot be spawned.");
		return;
	}



	UE_LOG(LogTemp, Warning, TEXT("Try to read data from MongoDB."));

	// TODO uncomment because of crash	
//  TArray<FString> AllItemNames = DatabaseHandler->Execute_GetAllItemNamesFromDB(Cast<UObject>(DatabaseHandler));
//	TArray<FString> AllIDs = DatabaseHandler->Execute_GetAllIDsFromDB(Cast<UObject>(DatabaseHandler));
//	//TArray<FString> AllThumbnailPaths = DatabaseHandler->Execute_GetAllItemNamesFromDB(Cast<UObject>(DatabaseHandler));
//
//	if (AllIDs.Num() != AllItemNames.Num())
//		return;
//
//	if (AllIDs.Num() == 0)
//		UE_LOG(LogTemp, Warning, TEXT("Cannot read data from MongoDB. Maybe offline."));
//
//	for (int i = 0; i < AllItemNames.Num(); i++)
//	{
//		IDToGUIParts.Add(AllIDs[i], TPair<FString, FString>(AllItemNames[i], ""));
//	}



}

// Called every frame
void ARAssetLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARAssetLoader::SpawnAssetFromCache(const FString FileName, const FVector Location, const FRotator Rotation)
{
	FString PathToFileName = FPaths::GameContentDir().Append("Cache/").Append(FileName);

	SpawnAsset(PathToFileName, Location, Rotation);
}

//Spawns a new RefillObject with the given location and rotation and attach the mesh from the .uasset-file under the given pathname
//path has to be like: /Game/Meshes/Mug.Mug while 'Game' equals the 'Content'-folder and the asset-file is 'Mug.uasset' in the folder 'Meshes' (which is located in the 'Content'-folder)
void ARAssetLoader::SpawnAsset(const FString Path, const FVector Location, const FRotator Rotation) {
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
	FActorSpawnParameters SpawnInfo;

	//Create a new RefillObject in the world
	ARRefillObject* RefillObj = GetWorld()->SpawnActor<ARRefillObject>(Location, Rotation, SpawnInfo);

	//Path to the mesh for the RefillObject
	RefillObj->LoadRefillObject(Path);

	SpawnedItem = RefillObj;
	SpawnedItem->SetMobility(EComponentMobility::Movable);

}

//For testing purposes. Method loads uassets and then spawns the mesh
void ARAssetLoader::SpawnTest() {

	//*** For working while it still crashes
	SpawnAsset(FPaths::GameContentDir().Append("Cache/") + "Mug.uasset", SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
	return;
	//*****


	TArray<FString> KeyArray;
	IDToGUIParts.GenerateKeyArray(KeyArray);
	TArray<TPair<FString, FString>> ValueArray;
	IDToGUIParts.GenerateValueArray(ValueArray);

	if (KeyArray.Num() == 0 || ValueArray.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No data read from MongoDB"));
		return;
	}

	FString ItemName = ValueArray[0].Key;
	FString FolderName = ItemName + "_" + KeyArray[0];
	FString Path = FPaths::GameContentDir().Append("Cache/").Append("Items/").Append(FolderName).Append("SM_" + ItemName + ".uasset");

	if (FPaths::FileExists(Path))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, "File already in Cache.");
	}
	else
	{
		IRIDatabase* DatabaseHandler = Cast<IRIDatabase>(DatabaseInterface);

		if (DatabaseHandler == nullptr)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red,
				"The Database Interface is not set, so the object cannot be spawned.");
			return;
		}

		// Load file from database
		bool bFileLoaded = DatabaseHandler->Execute_LoadItemFromDBIntoCache(Cast<UObject>(DatabaseHandler), ItemName);

		if (!bFileLoaded)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red,
				"The requested file cannot be loaded.");
			return;
		}
	}

	if (SpawnPoint)
		SpawnAssetFromCache("Items/" + FolderName + "/SM_" + ItemName + ".uasset", SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation());
	else
		SpawnAssetFromCache("Items/" + FolderName + "/SM_" + ItemName + ".uasset", FVector(100.0, -30.0, 100.0), FRotator(0.0, 0.0, 0.0));

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, "Object spawned.");
}

AStaticMeshActor* ARAssetLoader::GetSpawnedItem()
{
	return SpawnedItem;
}
