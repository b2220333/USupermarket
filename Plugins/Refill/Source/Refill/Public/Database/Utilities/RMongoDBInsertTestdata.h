// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

#pragma warning(push)
#pragma warning(disable : 4668)

#include <bsoncxx/json.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#pragma warning(pop)

#include "GameFramework/Actor.h"
#include "RMongoDBInsertTestdata.generated.h"

UCLASS()
class REFILL_API ARMongoDBInsertTestdata : public AActor
{
	GENERATED_BODY()

public:
	// The foldername "Content/Items/" + foldername to be uploaded
	UPROPERTY(EditAnywhere, Category = "RMongoDBInsertTestdata")
		FString FolderNameToUpload;

	// The IPAdress of the mongodb server
	UPROPERTY(EditAnywhere, Category = "RMongoDB")
		FString IPAdress;

	// The Port of the mongodb server
	UPROPERTY(EditAnywhere, Category = "RMongoDB")
		FString Port;

	// Sets default values for this actor's properties
	ARMongoDBInsertTestdata();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Serializes a file into a binary string
	std::string SerializeFromFile(FString Path);

	// Create a file with serialized content
	bool SerializeIntoFile(std::string Path, std::string SerializedFile);
};
