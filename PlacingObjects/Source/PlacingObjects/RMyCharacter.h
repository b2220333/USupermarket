// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include "GameFramework/Character.h"
#include "RMyCharacter.generated.h"

UCLASS()
class PLACINGOBJECTS_API ARMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMyCharacter();

	//UPROPERTY(EditAnywhere)
	//	AActor* Item;

	//UPROPERTY(EditAnywhere)
	//	float RaycastRange;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
protected:

	//// Handles moving forward/backward
	//void MoveForward(const float Val);

	//// Handles strafing Left/Right
	//void MoveRight(const float Val);

	//std::pair<FHitResult, bool> StartRaytrace();

	//// Places a single item
	//void PlaceSingleItem();

	//// Places a row of items
	//void PlaceRowItems();

	//// Places items to fill the available space
	//void FillItems();

private:
	// Helper function: Places an item to a specific position
	//void PlaceItemAtPosition(FVector Position);	

	//UStaticMeshComponent* GetStaticMesh(AActor* Actor);
};
