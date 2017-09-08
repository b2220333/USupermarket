// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "RMongoDB.h"

#include <fstream>

#pragma warning(push)
#pragma warning(disable : 4668)

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/json.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/exception/exception.hpp>

#pragma warning(pop)

// Sets default values
ARMongoDB::ARMongoDB()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	IPAdress = "localhost";
	Port = "27017";
}

// Called when the game starts or when spawned
void ARMongoDB::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARMongoDB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ARMongoDB::SerializeIntoFile(const std::string FilePath, const std::string SerializedFile)
{
	std::ofstream Output(FilePath, std::ios::binary);

	if (Output.is_open() && Output.good())
	{
		Output << SerializedFile;
		Output.close();
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Ofstream is not open"));

	return false;
}

bool ARMongoDB::LoadItemFromDBIntoCache_Implementation(const FString& ItemFolderName)
{
	try {
		/// Connect to MongoDB
		if (IPAdress.IsEmpty() || Port.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Reconfigure the Port and IP-Adress"));
			return false;
		}
		FString MongoDBAdress = "mongodb://" + IPAdress + ":" + Port;

		mongocxx::instance Instance{}; // This should be done only once.
		mongocxx::uri Uri(TCHAR_TO_UTF8(*MongoDBAdress));
		mongocxx::client Client(Uri);

		mongocxx::database Database = Client["UE4DB"];
		mongocxx::collection Collection = Database["UE4Col"];


		// READ DOCUMENT FROM MONGODB
		auto Cursor = Collection.find(
			bsoncxx::builder::stream::document{}
			<< "Name" << TCHAR_TO_UTF8(*ItemFolderName)
			<< bsoncxx::builder::stream::finalize);

		for (auto FoundDoc : Cursor)
		{
			/// READ Item ID
			bsoncxx::document::element ElementID = FoundDoc["_id"];
			if (!ElementID || ElementID.type() != bsoncxx::type::k_oid) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			FString ID = ElementID.get_oid().value.to_string().c_str();
			UE_LOG(LogTemp, Warning, TEXT("ID: %s"), *ID);


			/// READ Item Name
			bsoncxx::document::element ElementItemName = FoundDoc["Name"];
			if (!ElementItemName || ElementItemName.type() != bsoncxx::type::k_utf8) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			FString Name = ElementItemName.get_utf8().value.to_string().c_str();
			UE_LOG(LogTemp, Warning, TEXT("ItemName: %s"), *Name);

			FString ItemFolderPath = CreateFolderHierarchy(Name + "_" + ID);

			/// Read Filearray
			bsoncxx::document::element ElementFiles = FoundDoc["Files"];
			if (!ElementFiles || ElementFiles.type() != bsoncxx::type::k_array) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE - Files"));
				continue;
			}
			auto FileArray{ ElementFiles.get_array().value };
			for (auto FileDocument : FileArray) {

				if (FileDocument.type() != bsoncxx::type::k_document) {
					UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE - FileDocument"));
					continue;
				}
				bsoncxx::document::view TempDocument{ FileDocument.get_document() };

				/// Read FileName
				if (!TempDocument["FileName"] || TempDocument["FileName"].type() != bsoncxx::type::k_utf8) {
					UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE - FileName"));
					continue;
				}
				FString FileName = TempDocument["FileName"].get_utf8().value.to_string().c_str();

				if (!TempDocument["Binary"] || TempDocument["Binary"].type() != bsoncxx::type::k_utf8) {
					UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE - FileName"));
					continue;
				}
				std::string BinaryFile = TempDocument["Binary"].get_utf8().value.to_string();

				if (BinaryFile.empty() || FileName.IsEmpty())
				{
					UE_LOG(LogTemp, Warning, TEXT("Read data is empty"));
					continue;
				}

				/// WRITE SERIALIZED MESH

				FString CompleteFilePath = ItemFolderPath + FileName;
				UE_LOG(LogTemp, Warning, TEXT("CompleteFilePath: %s"), *CompleteFilePath);

				if (!SerializeIntoFile(TCHAR_TO_UTF8(*CompleteFilePath), BinaryFile))
				{
					UE_LOG(LogTemp, Warning, TEXT("Mesh Serialization failed!"));
				}
			}
		}
		return true;
	}
	catch (const bsoncxx::exception& BSONException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = BSONException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("BSON-Exception: %s"), *Exception);
		return false;
	}
	catch (const mongocxx::exception &MongoDBException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = MongoDBException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("MongoDB-Exception: %s"), *Exception);
		return false;
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("An Error occured"));
		return false;
	}
}

TArray<FString> ARMongoDB::GetAllItemNamesFromDB_Implementation()
{
	TArray<FString> AllItemNames;
	try {
		if (IPAdress.IsEmpty() || Port.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Reconfigure the Port and IP-Adress"));
			return AllItemNames;
		}
		FString MongoDBAdress = "mongodb://" + IPAdress + ":" + Port;

		mongocxx::instance Instance{}; // This should be done only once.
		mongocxx::uri Uri(TCHAR_TO_UTF8(*MongoDBAdress));
		mongocxx::client Client(Uri);

		mongocxx::database Database = Client["UE4DB"];
		mongocxx::collection Collection = Database["UE4Col"];

		// Create the query filter
		auto SearchFilter = bsoncxx::builder::stream::document{} << bsoncxx::builder::stream::finalize;

		// Create the find options with the projection
		mongocxx::options::find SearchOptions{};
		SearchOptions.sort(bsoncxx::builder::stream::document{} << "Name" << 1 << bsoncxx::builder::stream::finalize);
		SearchOptions.projection(bsoncxx::builder::stream::document{} << "Name" << 1 << bsoncxx::builder::stream::finalize);

		// READ DOCUMENT FROM MONGODB
		auto Cursor = Collection.find(SearchFilter.view(), SearchOptions);

		for (auto Doc : Cursor) {
			/*
			bsoncxx::document::element ElementID = Doc["_id"];
			if (ElementID.type() != bsoncxx::type::k_oid) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			ID = ElementID.get_oid().value.to_string().c_str();
			*/


			bsoncxx::document::element ElementFileName = Doc["Name"];
			if (ElementFileName.type() != bsoncxx::type::k_utf8) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			FString ItemName = ElementFileName.get_utf8().value.to_string().c_str();

			AllItemNames.Add(ItemName);
		}

		// Debug
		//for (auto Part : AllItemNames)
		//UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *Part.Key);

		return AllItemNames;
	}
	catch (const bsoncxx::exception& BSONException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = BSONException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("BSON-Exception: %s"), *Exception);
		return AllItemNames;
	}
	catch (const mongocxx::exception &MongoDBException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = MongoDBException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("MongoDB-Exception: %s"), *Exception);
		return AllItemNames;
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("An Error occured"));
		return AllItemNames;
	}
}
TArray<FString> ARMongoDB::GetAllIDsFromDB_Implementation()
{
	TArray<FString> AllIDs;
	try {
		if (IPAdress.IsEmpty() || Port.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Reconfigure the Port and IP-Adress"));
			return AllIDs;
		}
		FString MongoDBAdress = "mongodb://" + IPAdress + ":" + Port;

		mongocxx::instance Instance{}; // This should be done only once.
		mongocxx::uri Uri(TCHAR_TO_UTF8(*MongoDBAdress));
		mongocxx::client Client(Uri);

		mongocxx::database Database = Client["UE4DB"];
		mongocxx::collection Collection = Database["UE4Col"];

		// Create the query filter
		auto SearchFilter = bsoncxx::builder::stream::document{} << bsoncxx::builder::stream::finalize;

		// Create the find options with the projection
		mongocxx::options::find SearchOptions{};
		SearchOptions.sort(bsoncxx::builder::stream::document{} << "Name" << 1 << bsoncxx::builder::stream::finalize);
		SearchOptions.projection(bsoncxx::builder::stream::document{} << "Name" << 1 << bsoncxx::builder::stream::finalize);

		// READ DOCUMENT FROM MONGODB
		auto Cursor = Collection.find(SearchFilter.view(), SearchOptions);

		for (auto Doc : Cursor) {
			bsoncxx::document::element ElementID = Doc["_id"];
			if (ElementID.type() != bsoncxx::type::k_oid) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			FString ID = ElementID.get_oid().value.to_string().c_str();


			/*
			bsoncxx::document::element ElementFileName = Doc["Name"];
			if (ElementFileName.type() != bsoncxx::type::k_utf8) {
				UE_LOG(LogTemp, Warning, TEXT("WRONG TYPE"));
				continue;
			}
			ItemName = ElementFileName.get_utf8().value.to_string().c_str();
			*/

			AllIDs.Add(ID);
		}

		// Debug
		//for (auto Part : AllItemNames)
		//UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *Part.Key);

		return AllIDs;
	}
	catch (const bsoncxx::exception& BSONException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = BSONException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("BSON-Exception: %s"), *Exception);
		return AllIDs;
	}
	catch (const mongocxx::exception &MongoDBException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = MongoDBException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("MongoDB-Exception: %s"), *Exception);
		return AllIDs;
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("An Error occured"));
		return AllIDs;
	}
}

bool ARMongoDB::VerifyOrCreateDirectory(const FString& NewDirectory) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*NewDirectory))
	{
		PlatformFile.CreateDirectory(*NewDirectory);

		if (!PlatformFile.DirectoryExists(*NewDirectory))
		{
			return false;
		}
	}
	return true;
}

/// CREATE FOLDER HIERARCHY
FString ARMongoDB::CreateFolderHierarchy(const FString& ItemFolderName) {

	/// CREATE CACHE FOLDER

	FString CachePath = FPaths::ConvertRelativePathToFull(
		FPaths::GameContentDir()).Append("Cache/");

	if (VerifyOrCreateDirectory(CachePath)) {
		UE_LOG(LogTemp, Warning, TEXT("Cachefolder created!"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cachefolder cannot be created! EXIT"));
		return "";
	}

	/// CREATE CACHE/ITEMS FOLDER

	FString ItemPath = CachePath + "Items/";

	if (VerifyOrCreateDirectory(ItemPath)) {
		UE_LOG(LogTemp, Warning, TEXT("Itemfolder created!"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Itemfolder cannot be created! EXIT"));
		return "";
	}

	/// CREATE CACHE/ITEMS/ITEMNAME FOLDER

	ItemPath += ItemFolderName + "/";// TODO: +"_" + ObjectID;

	if (VerifyOrCreateDirectory(ItemPath)) {
		UE_LOG(LogTemp, Warning, TEXT("Itemfolder created!"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Itemfolder cannot be created! EXIT"));
		return "";
	}
	return ItemPath;
}