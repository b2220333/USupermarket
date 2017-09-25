// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "RRefillObject.h"

// Sets default values
ARRefillObject::ARRefillObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

ARRefillObject::ARRefillObject(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

// Called when the game starts or when spawned
void ARRefillObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARRefillObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Loads the Object (which has to be a UStaticMesh) from the given asset path
void ARRefillObject::LoadRefillObject(const FString AssetPath) {
	if (AssetPath.IsEmpty()) return;

	FString Path = AssetPath;
	FString TrimmedPath = "";
	FString RelativePath = "";
	FString FileName = "";
	FString UnusedDummy = "";

	Path.Split(TEXT("Content/"), &UnusedDummy, &TrimmedPath, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	TrimmedPath.Split(TEXT("/"), &RelativePath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FileName.Split(TEXT("."), &FileName, &UnusedDummy, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	FString MeshPath = TEXT("StaticMesh'/Game/") + RelativePath + TEXT("/") + FileName + TEXT(".") + FileName + TEXT("'");
	MeshPath.Replace(TEXT("//"), TEXT("/")); // replace double slashes

	UE_LOG(LogTemp, Log, TEXT("%s: Used Mesh: %s"), *FString(__FUNCTION__), *MeshPath);

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
	if (!LoadedMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: This file is not found or does not contain a static mesh"), *FString(__FUNCTION__));
		return;
	}
	SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetStaticMesh(LoadedMesh);
}