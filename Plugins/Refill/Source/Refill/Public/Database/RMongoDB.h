// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>

#include "RIDatabase.h"

#include "GameFramework/Actor.h"
#include "RMongoDB.generated.h"

UCLASS()
class REFILL_API ARMongoDB : public AActor, public IRIDatabase
{
	GENERATED_BODY()

public:
	// The IPAdress of the mongodb server
	UPROPERTY(EditAnywhere, Category = "RMongoDB")
		FString IPAdress;

	// The Port of the mongodb server
	UPROPERTY(EditAnywhere, Category = "RMongoDB")
		FString Port;

	// Sets default values for this actor's properties
	ARMongoDB();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// To be implemented by the IRIDatabase Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RMongoDB")
		bool LoadItemFromDBIntoCache(const FString& ItemName);
	bool LoadItemFromDBIntoCache_Implementation(const FString& ItemName);

	// To be implemented by the IRIDatabase Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RMongoDB")
		TArray<FString> GetAllItemNamesFromDB();
	TArray<FString> GetAllItemNamesFromDB_Implementation();

	// To be implemented by the IRIDatabase Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RMongoDB")
		TArray<FString> GetAllIDsFromDB();
	TArray<FString> GetAllIDsFromDB_Implementation();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Serializes a binary string into a File. 
	// IMPORTANT: the std::string type must not be changed.
	bool SerializeIntoFile(const std::string FilePath, const std::string SerializedFile);

	// Create the Folder Hierarchy for this Project
	FString CreateFolderHierarchy(const FString& ItemFolderName);

	// Creates a Directory if it does not exist
	bool VerifyOrCreateDirectory(const FString& NewDirectory) const;
};
