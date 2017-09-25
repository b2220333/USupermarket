// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "MyHUD.h"
#include "RMyCharacter.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::White,text, false)

// Sets default values
ARMyCharacter::ARMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Print controlls on screen 
	print("Left Mouse = Place one item");
	print("Shift + Left Mouse = Place a row of items");
	print("CTRL + Left Mouse = Fill surface with items");
	print("MouseWheel = Change Spacing (in combination with Shift or CTRL)");
	print("E = Open/Close Inventory");
	print("R = Rotate item");
	print("F = Toggle placing/selecting mode");
	print("1 = Decrease spacing steps for a smother spacing");
	print("2 = Increase spacing step");

	// Make sure the player doesn't collide with all the items
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void ARMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	CPlaceItemComponent = Cast<UCPlaceItem>(GetComponentByClass(UCPlaceItem::StaticClass()));
	if (CPlaceItemComponent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s: Could not find CPlaceItem component."), *FString(__FUNCTION__));
	}
	else CPlaceItemComponent->SetupKeyBindings(PlayerInputComponent); // Setup key bindings for PlaceItem component 
}

