// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RIDatabase.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class URIDatabase : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * Interface for communication to the MongoDB
 */
class REFILL_API IRIDatabase
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RIDatabase")
		bool LoadItemFromDBIntoCache(const FString& ItemName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RIDatabase")
		TArray<FString> GetAllItemNamesFromDB();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RIDatabase")
		TArray<FString> GetAllIDsFromDB();
};
