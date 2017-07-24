// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacingObjects.h"
//#include "Runtime/PerfCounters/Public/PerfCountersModule.h"
//#include "Runtime/Json/Public/Json.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
//#include "Runtime/Core/Public/Templates/SharedPointer.h"

#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "SavegameManager.h"



// Sets default values
ASavegameManager::ASavegameManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASavegameManager::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCharacter != nullptr)
	{
		UInputComponent* PlayerInputComponent = PlayerCharacter->InputComponent;

		PlayerInputComponent->BindAction("SaveMap", IE_Pressed, this, &ASavegameManager::SaveGame);
		PlayerInputComponent->BindAction("LoadMap", IE_Pressed, this, &ASavegameManager::LoadGame);
	}




	// Look for Place Component
	TArray<UActorComponent*> Components;
	PlayerCharacter->GetComponents(Components);

	for (auto& Comp : Components)
	{
		PlaceComponent = Cast<UCPlaceItem>(Comp);

		if (PlaceComponent != nullptr)
		{
			break;
		}
	}

}

// Called every frame
void ASavegameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ASavegameManager::SaveGame()
{
	//TArray<AActor*> ItemList = PlaceComponent->GetListOfItems();
	// TArray<AActor*> ItemList = PlaceComponent->ListOfRefillTitemsPlacedInWorld;

	// Create a writer and hold it in this FString
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	for (auto & elem : PlaceComponent->ListOfRefillTitemsPlacedInWorld)
	{
		// Create a new JSon opbject for that item
		TSharedPtr<FJsonObject> JsonItem = MakeShareable(new FJsonObject);

		JsonItem->SetStringField("Name", *elem->GetName());

		// Write Tags to an array **************
		TArray<TSharedPtr<FJsonValue>> TagValues;
		for (auto& tag : elem->Tags)
		{
			TagValues.Add(MakeShareable(new FJsonValueString(tag.ToString())));
		}

		JsonItem->SetArrayField("Tags", TagValues); // Set array to JsonItem
		//***************************************		


		// Create a Position Json object ********
		TSharedPtr<FJsonObject> JsonItemPosition = MakeShareable(new FJsonObject);
		JsonItemPosition->SetNumberField("X", elem->GetActorLocation().X);
		JsonItemPosition->SetNumberField("Y", elem->GetActorLocation().Y);
		JsonItemPosition->SetNumberField("Z", elem->GetActorLocation().Z);

		FString ItemPositionString;
		TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR> > > WriterPosition = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ItemPositionString);
		FJsonSerializer::Serialize(JsonItemPosition.ToSharedRef(), WriterPosition);

		JsonItem->SetObjectField("Position", JsonItemPosition); // Add the position to our JsonItem
		// **************************************


		// Create Rotation Json object **********
		TSharedPtr<FJsonObject> JsonItemRotation = MakeShareable(new FJsonObject);
		JsonItemRotation->SetNumberField("Pitch", elem->GetActorRotation().Pitch);
		JsonItemRotation->SetNumberField("Yaw", elem->GetActorRotation().Yaw);
		JsonItemRotation->SetNumberField("Roll", elem->GetActorRotation().Roll);

		FString ItemRotationString;
		TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR> > > WriterRotation = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ItemRotationString);
		FJsonSerializer::Serialize(JsonItemPosition.ToSharedRef(), WriterRotation);

		JsonItem->SetObjectField("Rotation", JsonItemRotation);
		// **************************************

		//  Something else(scaling etc) - Use the code above as template
		// ...
		// ...
		// ...
		// **************************************

		AStaticMeshActor* StaticMeshActor = Cast <AStaticMeshActor>(elem);
		if (StaticMeshActor != nullptr)
		{
			FString AssetPath = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh()->GetFullName();
			AssetPath.RemoveFromStart("StaticMesh ");
			JsonItem->SetStringField("AssetPath", AssetPath); // TODO Asset Path should be relative to the game folder
		}

		// Write JsonItem to final Json Object
		FString ItemTagString;
		TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR> > > WriterItem = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ItemTagString);
		FJsonSerializer::Serialize(JsonItem.ToSharedRef(), WriterItem);

		// Add JsonItem to JSonObject
		JsonObject->SetObjectField(*elem->GetName(), JsonItem);
	}

	//JsonObject->SetStringField("Name", "Super Sword");
	//JsonObject->SetNumberField("Damage", 15);
	//JsonObject->SetNumberField("Weight", 3);

	FString JsonObjectString;

	TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR> > > Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonObjectString);

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Warning, TEXT("Writing to file:\n %s"), *JsonObjectString);

	// File writing ******
	FString SaveDirectory = FPaths::GameContentDir().Append("Cache/");
	FString FileName = FString("UE4TestJSON.json");
	bool AllowOverwriting = true;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

		if (PlatformFile.FileExists(*AbsoluteFilePath)) PlatformFile.DeleteFile(*AbsoluteFilePath);
		// Allow overwriting or file doesn't already exist
		if (AllowOverwriting || PlatformFile.FileExists(*AbsoluteFilePath) == false)
		{
			FFileHelper::SaveStringToFile(JsonObjectString, *AbsoluteFilePath);
		}
	}

}

void ASavegameManager::LoadGame()
{
	if (AssetLoader == nullptr)
	{
		// TODO Complain
		return;
	}

	// Clear level items
	//TArray<AActor*>ItemList = PlaceComponent->GetListOfItems();
	// TArray<AActor*>ItemList = PlaceComponent->ListOfRefillTitemsPlacedInWorld;


	for (auto& item : PlaceComponent->ListOfRefillTitemsPlacedInWorld)
	{
		item->Destroy();
	}
	PlaceComponent->ListOfRefillTitemsPlacedInWorld.Empty();

	// ******************

	FString JsonObjectString;

	// Reading writing ******
	FString SaveDirectory = FPaths::GameContentDir().Append("Cache/");
	FString FileName = FString("UE4TestJSON.json");
	bool AllowOverwriting = true;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Get absolute file path
	FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.FileExists(*AbsoluteFilePath)) //PlatformFile.CreateDirectoryTree(*SaveDirectory)
	{
		FFileHelper::LoadFileToString(JsonObjectString, *AbsoluteFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("File not found: %s"), *AbsoluteFilePath);
	}
	// *****************************************************






	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef< TJsonReader<> > ObjectReader = TJsonReaderFactory<>::Create(JsonObjectString);

	if (FJsonSerializer::Deserialize(ObjectReader, JsonObject))
	{
		for (auto& curItem : JsonObject->Values)
		{
			UE_LOG(LogTemp, Warning, TEXT("Loading item %s"), *curItem.Key);
			auto& RefillItem = *curItem.Value->AsObject();

			FString name = RefillItem.GetStringField("Name");

			TArray<FName> ItemTags;
			for (auto& tag : RefillItem.GetArrayField("Tags"))
			{
				ItemTags.Add(FName(*tag.Get()->AsString()));
			}

			auto& ItemPosition = *RefillItem.GetObjectField("Position");
			float PosX = ItemPosition.GetNumberField("X");
			float PosY = ItemPosition.GetNumberField("Y");
			float PosZ = ItemPosition.GetNumberField("Z");
			FVector PositionVector = FVector(PosX, PosY, PosZ);

			auto& ItemRotation = *RefillItem.GetObjectField("Rotation");
			float Pitch = ItemRotation.GetNumberField("Pitch");
			float Yaw = ItemRotation.GetNumberField("Yaw");
			float Roll = ItemRotation.GetNumberField("Roll");
			FRotator Rotation = FRotator(Pitch, Yaw, Roll);

			FString AssetPath = RefillItem.GetStringField("AssetPath");

			PlaceItem(AssetPath, PositionVector, Rotation, ItemTags);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialization failed"));
	}
}

void ASavegameManager::PlaceItem(FString AssetPath, FVector Location, FRotator Rotation, TArray<FName> ItemTags)
{
	FString CachePath;
	FActorSpawnParameters SpawnInfo;
	ARRefillObject* RefillObj = GetWorld()->SpawnActor<ARRefillObject>(Location, Rotation, SpawnInfo);

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *AssetPath));

	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("This file is not found or does not contain a static mesh"));
		return;
	}

	RefillObj->SetMobility(EComponentMobility::Movable);
	RefillObj->GetStaticMeshComponent()->SetStaticMesh(LoadedMesh);

	for (auto & tag : ItemTags)
	{
		RefillObj->Tags.Add(tag);
	}

	RefillObj->GetStaticMeshComponent()->SetCollisionProfileName("OverlapOnlyPawn");
	RefillObj->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RefillObj->GetStaticMeshComponent()->SetSimulatePhysics(true);
	RefillObj->GetStaticMeshComponent()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RefillObj->GetStaticMeshComponent()->bGenerateOverlapEvents = true;

	//PlaceComponent->GetListOfItems().Add(RefillObj);
	PlaceComponent->ListOfRefillTitemsPlacedInWorld.Add(RefillObj);

}

