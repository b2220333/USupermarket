// Fill out your copyright notice in the Description page of Project Settings.


#include "Refill.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "TagStatics.h"

#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "SavegameManager.h"



// Sets default values
ASavegameManager::ASavegameManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SavegameFileName = FString("UE4TestJSON.json");
	SaveDirectory = FPaths::GameContentDir().Append("Cache/");
}

// Called when the game starts or when spawned
void ASavegameManager::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<ARMyCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	check(PlayerCharacter); // Assert the existence of the PlayerCharacter

	UInputComponent* PlayerInputComponent = PlayerCharacter->InputComponent;

	// Binding keys for loading and saving
	PlayerInputComponent->BindAction("SaveMap", IE_Pressed, this, &ASavegameManager::SaveGame);
	PlayerInputComponent->BindAction("LoadMap", IE_Pressed, this, &ASavegameManager::LoadGame);

	// Look for PlaceComponent
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

	// Find ItemManager
	for (TActorIterator<AItemManager>Itr(GetWorld()); Itr; ++Itr)
	{
		ItemManager = *Itr;
		break;
	}

	check(ItemManager); // Assert ItemManager
}

// Called every frame
void ASavegameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ASavegameManager::SaveGame()
{
	if (ItemManager == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s %s: Saving file failed. No ItemManager instance was found."), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return;
	}

	// Create a writer and hold it in this FString
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	for (auto & elem : ItemManager->ListOfRefillTitemsPlacedInWorld)
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

		if (FTagStatics::HasKeyValuePair(elem, "Refill", "Hookable", "True")) { // TODO hardcoded tag
			ARRefillObject* CastedObject = Cast<ARRefillObject>(elem);
			if (CastedObject != nullptr) {
				UActorComponent* Hook = ItemManager->FindHookOfItem(CastedObject);
				if (Hook != nullptr) {
					FString HookName = Hook->GetName();
					JsonItem->SetStringField("HookFName", *HookName);
				}
			}
		}

		//  Something else(scaling etc) - Use the code above as template
		// ...
		// ...
		// ...
		// **************************************

		AStaticMeshActor* StaticMeshActor = Cast <AStaticMeshActor>(elem);
		if (StaticMeshActor != nullptr)
		{
			// Save file name of mesh
			FString MeshName = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh()->GetName();
			JsonItem->SetStringField("MeshFile", MeshName);
		}

		// Write JsonItem to final Json Object
		FString ItemTagString;
		TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> WriterItem = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ItemTagString);
		FJsonSerializer::Serialize(JsonItem.ToSharedRef(), WriterItem);

		// Add JsonItem to JSonObject
		JsonObject->SetObjectField(*elem->GetName(), JsonItem);
	}

	FString JsonObjectString;

	TSharedRef< TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR> > > Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonObjectString);

	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("%s: Writing to file:\n %s"), *FString(__FUNCTION__), *JsonObjectString);

	// File writing ******
	bool AllowOverwriting = true;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = SaveDirectory + "/" + SavegameFileName;

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
		UE_LOG(LogTemp, Error, TEXT("%s: Couldn't find the asset loader. Could not load the file."), *FString(__FUNCTION__));
		return;
	}

	if (ItemManager == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s %s: Loading failed. No ItemManager instance was found."), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return;
	}

	// Clear level
	for (auto& item : ItemManager->ListOfRefillTitemsPlacedInWorld)
	{
		item->Destroy();
	}
	ItemManager->ListOfRefillTitemsPlacedInWorld.Empty();
	// ******************

	FString JsonObjectString;

	// Reading json file ******
	// FString SaveDirectory = FPaths::GameContentDir().Append("Cache/");
	// FString FileName = FString("UE4TestJSON.json");
	bool AllowOverwriting = true;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Get absolute file path
	FString AbsoluteFilePath = SaveDirectory + "/" + SavegameFileName;

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.FileExists(*AbsoluteFilePath)) //PlatformFile.CreateDirectoryTree(*SaveDirectory)
	{
		FFileHelper::LoadFileToString(JsonObjectString, *AbsoluteFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: File not found: %s"), *FString(__FUNCTION__), *AbsoluteFilePath);
	}
	// *****************************************************

	// Deserialization of json file
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef< TJsonReader<> > ObjectReader = TJsonReaderFactory<>::Create(JsonObjectString);

	if (FJsonSerializer::Deserialize(ObjectReader, JsonObject))
	{
		for (auto& curItem : JsonObject->Values)
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Loading item %s"), *FString(__FUNCTION__), *curItem.Key);
			auto& RefillItem = *curItem.Value->AsObject();

			// Read name of item
			FString name = RefillItem.GetStringField("Name");

			// Read tags
			TArray<FName> ItemTags;
			for (auto& tag : RefillItem.GetArrayField("Tags"))
			{
				ItemTags.Add(FName(*tag.Get()->AsString()));
			}
			
			// Read item position
			auto& ItemPosition = *RefillItem.GetObjectField("Position");
			float PosX = ItemPosition.GetNumberField("X");
			float PosY = ItemPosition.GetNumberField("Y");
			float PosZ = ItemPosition.GetNumberField("Z");
			FVector PositionVector = FVector(PosX, PosY, PosZ);

			// Read item rotation
			auto& ItemRotation = *RefillItem.GetObjectField("Rotation");
			float Pitch = ItemRotation.GetNumberField("Pitch");
			float Yaw = ItemRotation.GetNumberField("Yaw");
			float Roll = ItemRotation.GetNumberField("Roll");
			FRotator Rotation = FRotator(Pitch, Yaw, Roll);

			// Read name of mesh
			FString MeshFile = RefillItem.GetStringField("MeshFile");

			// Spawn item
			ARRefillObject* PlacedRefillObject = PlaceItem(MeshFile, PositionVector, Rotation, ItemTags);

			// Check if it an item on a hook
			if (PlacedRefillObject != nullptr && PlacedRefillObject->ObjectInfo.bCanBeHookedUp) {

				FName HookName = FName(*RefillItem.GetStringField("HookFName"));

				if (HookName.IsValid() && ItemManager->HooknamesToHookComponent.Contains(HookName))
					// Add the item to the hook
					ItemManager->AddItemToHook(ItemManager->HooknamesToHookComponent[HookName], PlacedRefillObject);
			}

			// Clear AssetLoader item
			AssetLoader->CurrentObject = nullptr;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Deserialization failed"), *FString(__FUNCTION__));
	}

	// Clear ItemTemplate
	PlaceComponent->ItemTemplate = nullptr;
}

ARRefillObject* ASavegameManager::PlaceItem(FString AssetName, FVector Location, FRotator Rotation, TArray<FName> ItemTags)
{
	ARRefillObject* RefillObj = AssetLoader->SpawnAsset(AssetName, Location, Rotation);

	// Physics and collision
	RefillObj->SetMobility(EComponentMobility::Movable);
	RefillObj->GetStaticMeshComponent()->SetCollisionProfileName("OverlapOnlyPawn");
	RefillObj->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RefillObj->GetStaticMeshComponent()->SetSimulatePhysics(true);
	RefillObj->GetStaticMeshComponent()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RefillObj->GetStaticMeshComponent()->bGenerateOverlapEvents = true;
	// *** *** *** *** *** ***

	for (auto & tag : ItemTags)
	{
		if (RefillObj->Tags.Contains(tag) == false) RefillObj->Tags.Add(tag); // Only add tag if not already existing
	}

	ItemManager->ListOfRefillTitemsPlacedInWorld.Add(RefillObj);

	return RefillObj;
}

