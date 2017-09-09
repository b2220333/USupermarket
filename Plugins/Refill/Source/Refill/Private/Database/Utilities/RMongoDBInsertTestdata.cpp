// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "RMongoDBInsertTestdata.h"

#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <FileManagerGeneric.h>

#pragma warning(push)
#pragma warning(disable : 4668)

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <bsoncxx/exception/exception.hpp>

#include <mongocxx/exception/exception.hpp>

#pragma warning(pop)

// Sets default values
ARMongoDBInsertTestdata::ARMongoDBInsertTestdata()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FolderNameToUpload = "";
	IPAdress = "localhost";
	Port = "27017";
}

// Called when the game starts or when spawned
void ARMongoDBInsertTestdata::BeginPlay()
{
	Super::BeginPlay();

	if (FolderNameToUpload.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Reconfigure the FolderNameToUpload"));
		return;
	}

	FString ContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());
	FString MeshDirectory = ContentDirectory + "Items/" + FolderNameToUpload + "/";

	/// Find Files in MeshDirectory
	TArray<FString> FoundFiles;
	FFileManagerGeneric::Get().FindFiles(FoundFiles, *MeshDirectory, *FString("uasset"));

	if (FoundFiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Files in Diectory found: %s"), *MeshDirectory);
		return;
	}

	/// Serialize Assets into Map
	TMap<FString, std::string> FileMap;

	for (FString FileName : FoundFiles) {
		UE_LOG(LogTemp, Warning, TEXT("FoundFile: %s"), *FileName);

		std::string SerializedFile = SerializeFromFile(MeshDirectory + FileName);
		if (SerializedFile.empty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Mesh was not found"), *FileName);
			continue;
		}

		FileMap.Add(FileName, SerializedFile);
	}

	try {

		/// Connect to MongoDB
		if (IPAdress.IsEmpty() || Port.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Reconfigure the Port and IP-Adress"));
			return;
		}

		FString MongoDBAdress = "mongodb://" + IPAdress + ":" + Port;

		mongocxx::instance Instance{}; // This should be done only once.
		mongocxx::uri Uri(TCHAR_TO_UTF8(*MongoDBAdress));
		mongocxx::client Client(Uri);

		mongocxx::database Database = Client["UE4DB"];
		mongocxx::collection Collection = Database["UE4Col"];

		/// Build Bson Document to upload
		bsoncxx::builder::stream::document Builder{};

		Builder << "Name" << TCHAR_TO_UTF8(*FolderNameToUpload);
		auto InArray = Builder << "Files" << bsoncxx::builder::stream::open_array;
		for (auto Entry : FileMap)
		{
			InArray << bsoncxx::builder::stream::open_document << "FileName" << TCHAR_TO_UTF8(*Entry.Key)
				<< "Binary" << bsoncxx::types::b_utf8{ Entry.Value } << bsoncxx::builder::stream::close_document;
		}
		InArray << bsoncxx::builder::stream::close_array;

		bsoncxx::document::value Document = Builder << bsoncxx::builder::stream::finalize;

		/// Upload Bson Document
		auto result = Collection.insert_one(std::move(Document));

		if (!result)
		{
			UE_LOG(LogTemp, Warning, TEXT("File cannot be inserted"));
		}
	}
	catch (const bsoncxx::exception& BSONException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = BSONException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("BSON-Exception: %s"), *Exception);
		return;
	}
	catch (const mongocxx::exception &MongoDBException) {
		// this exception is thrown, when a invalid json is parsed.
		std::string Debug = MongoDBException.what();
		FString Exception(Debug.c_str());
		UE_LOG(LogTemp, Warning, TEXT("MongoDB-Exception: %s"), *Exception);
		return;
	}
	catch (...) {
		UE_LOG(LogTemp, Warning, TEXT("An Error occured"));
		return;
	}
}

// Called every frame
void ARMongoDBInsertTestdata::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

std::string ARMongoDBInsertTestdata::SerializeFromFile(FString Path)
{
	UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *Path);

	std::ifstream Input(*Path, std::ios::binary);
	if (Input.is_open() && Input.good())
	{
		std::string s((std::istreambuf_iterator<char>(Input)), std::istreambuf_iterator<char>());
		return s;
	}

	UE_LOG(LogTemp, Warning, TEXT("Path does not lead to a valid file."));
	return "";
}

bool ARMongoDBInsertTestdata::SerializeIntoFile(std::string FileName, std::string SerializedFile)
{
	std::ofstream Output(FileName, std::ios::binary);

	if (Output.is_open() && Output.good())
	{
		Output << SerializedFile;
		Output.close();
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Ofstream is not open"));

	return false;
}
