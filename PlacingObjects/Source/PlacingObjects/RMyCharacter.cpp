// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "RMyCharacter.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 100000.0f,FColor::White,text, false)

// Sets default values
ARMyCharacter::ARMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

//	RaycastRange = 200;

}

// Called when the game starts or when spawned
void ARMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	print("Left Mouse = Place one item");
	print("Shift + Left Mouse = Place a row of items");
	print("CTRL + Left Mouse = Fill surface with items");

	// Attach the item to the player	
	//if (Item != nullptr) {
		//Item->AttachRootComponentToActor(this, FName(""), EAttachLocation::SnapToTarget, false);
		//Item->SetActorRelativeLocation(FVector(60, 0, 40));	
	//}
}

// Called every frame
void ARMyCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//std::pair<FHitResult, bool> HitResult = StartRaytrace();

	//if (HitResult.second != false) {

	//}
}

// Called to bind functionality to input
void ARMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//	// Set up gameplay key bindings
	//PlayerInputComponent->BindAxis("MoveForward", this, &ARMyCharacter::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &ARMyCharacter::MoveRight);
	//// Default Camera view bindings
	//PlayerInputComponent->BindAxis("CameraPitch", this, &ARMyCharacter::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("CameraYaw", this, &ARMyCharacter::AddControllerYawInput);

	 //PlayerInputComponent->BindAction("PlaceSingeItem", IE_Pressed,  this, &ARMyCharacter::PlaceSingleItem);
	 //PlayerInputComponent->BindAction("PlaceRowOfItems", IE_Pressed, this, &ARMyCharacter::PlaceRowItems);
	 //PlayerInputComponent->BindAction("FillWithItems", IE_Pressed, this, &ARMyCharacter::FillItems);
}



//void ARMyCharacter::MoveForward(const float Val)
//{
//	float SpeedFactor = 1.0f; // TODO Hardcoded
//
//	if ((Controller != nullptr) && (Val != 0.0f))
//	{
//		// Find out which way is forward
//		FRotator Rotation = Controller->GetControlRotation();
//		// Limit pitch when walking or falling
//		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
//		{
//			Rotation.Pitch = 0.0f;
//		}
//		// add movement in that direction
//		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
//		AddMovementInput(Direction, Val * SpeedFactor);
//	}
//}

//void ARMyCharacter::MoveRight(const float Val)
//{
//	float SpeedFactor = 1.0f; // TODO Hardcoded
//
//	if ((Controller != nullptr) && (Val != 0.0f))
//	{
//		// Find out which way is right
//		const FRotator Rotation = Controller->GetControlRotation();
//		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
//		// add movement in that direction
//		AddMovementInput(Direction, Val * SpeedFactor);
//	}
//}
//
//std::pair<FHitResult, bool> ARMyCharacter::StartRaytrace() {
//	FVector CamLoc;
//	FRotator CamRot;
//
//	Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
//	const FVector StartTrace = CamLoc; // trace start is the camera location
//	const FVector Direction = CamRot.Vector();
//	const FVector EndTrace = StartTrace + Direction * RaycastRange; // and trace end is the camera location + an offset in the direction
//
//														   //Collision parameters. The following syntax means that we don't want the trace to be complex
//	FCollisionQueryParams CollisionParameters;
//
//	//Hit contains information about what the raycast hit.
//	FHitResult Hit;
//
//
//	FCollisionQueryParams TraceParams;
//	TraceParams.TraceTag = FName("Box");
//	TraceParams.AddIgnoredActor(this);
//
//
//	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_Camera, TraceParams);
//	
//	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true, 3, 0, 1.f);
//
//	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
//	bool bIsBlockingVolume = false;
//
//	if (BlockingVol != nullptr) {
//		for (FName tag : BlockingVol->Tags) {
//			if (tag.IsEqual("Box")) { // Todo Hardcoded name
//				bIsBlockingVolume = true;
//				break;
//			}
//		}
//	}
//
//	return std::make_pair(Hit, bIsBlockingVolume);
//}
//
//void ARMyCharacter::PlaceItemAtPosition(FVector Position)
//{
//	FActorSpawnParameters Parameters;
//	Parameters.Template = Item;
//	AActor* Clone = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);
//	if (Clone == nullptr) return;
//
//	Clone->SetActorLocation(Position);
//}
//
//void ARMyCharacter::PlaceSingleItem()
//{
//	if (Item == nullptr) return;
//	
//	// *** Raytracing *** 
//	std::pair<FHitResult, bool> RaycastResults = StartRaytrace();
//	if (RaycastResults.second == false) return; // We didn't hit a blocking volume
//
//	FHitResult Hit = RaycastResults.first;
//	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
//
//	// Get position and bounds of the item
//	FVector ItemOrigin;
//	FVector ItemBoundExtend;
//	Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);
//
//	// Get Position and bound of the blocking volume
//	FVector BlockingVolumeOrigin;
//	FVector BlockingVolumeExtend;
//	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);
//	
//	// Calculating new position
//	FVector Position = Hit.ImpactPoint;
//	FVector NewPosition = FVector(Position.X, Position.Y, Position.Z + ItemBoundExtend.Z - (BlockingVolumeExtend.Z * 2));
//
//	PlaceItemAtPosition(NewPosition);
//}
//
//void ARMyCharacter::PlaceRowItems()
//{
//	// *** Raytracing *** 
//	std::pair<FHitResult, bool> RaycastResults = StartRaytrace();
//	if (RaycastResults.second == false) return; // We didn't hit a blocking volume
//
//	FHitResult Hit = RaycastResults.first;
//	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
//
//	// Get position and bounds of the item
//	FVector ItemOrigin;
//	FVector ItemBoundExtend;
//	Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);
//
//	// Get Position and bound of the blocking volume
//	FVector BlockingVolumeOrigin;
//	FVector BlockingVolumeExtend;
//	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);
//
//	// Calculating new positions
//	FVector Position = Hit.ImpactPoint;
//
//	int AmountOfItemsPossible = BlockingVolumeExtend.X / ItemBoundExtend.X;
//
//	UE_LOG(LogTemp, Warning, TEXT("Placing %i items"), AmountOfItemsPossible);
//
//	FVector StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, Position.Y, Position.Z + ItemBoundExtend.Z - (BlockingVolumeExtend.Z * 2));
//
//	for (size_t  i = 0; i < AmountOfItemsPossible; i++)
//	{
//		FVector NewPosition = FVector(StartVector.X + i * ItemBoundExtend.X * 2, StartVector.Y, StartVector.Z);
//		PlaceItemAtPosition(NewPosition);
//	}
//}
//
//void ARMyCharacter::FillItems() {
//	// *** Raytracing *** 
//	std::pair<FHitResult, bool> RaycastResults = StartRaytrace();
//	if (RaycastResults.second == false) return; // We didn't hit a blocking volume
//
//	FHitResult Hit = RaycastResults.first;
//	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
//
//	// Get position and bounds of the item
//	FVector ItemOrigin;
//	FVector ItemBoundExtend;
//	Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);
//
//	// Get Position and bound of the blocking volume
//	FVector BlockingVolumeOrigin;
//	FVector BlockingVolumeExtend;
//	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);
//
//	FVector Position = Hit.ImpactPoint;
//
//	int AmountOfItemsPossibleX = BlockingVolumeExtend.X / ItemBoundExtend.X;
//	int AmountOfItemsPossibleY = BlockingVolumeExtend.Y / ItemBoundExtend.Y;
//
//	UE_LOG(LogTemp, Warning, TEXT("Placing %i items"), AmountOfItemsPossibleX * AmountOfItemsPossibleY);
//
//	FVector StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, BlockingVolumeOrigin.Y - BlockingVolumeExtend.Y + ItemBoundExtend.Y, Position.Z + ItemBoundExtend.Z - (BlockingVolumeExtend.Z * 2));
//	
//	for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
//	{
//		for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
//		{
//			FVector NewPosition = FVector(StartVector.X + i * ItemBoundExtend.X * 2, StartVector.Y + j * ItemBoundExtend.Y * 2, StartVector.Z);
//			PlaceItemAtPosition(NewPosition);
//		}		
//	}
//}
//
//UStaticMeshComponent* ARMyCharacter::GetStaticMesh(AActor* Actor)
//{
//	for (auto Component : Actor->GetComponents())
//	{
//		if (Component->GetName().Contains("StaticMeshComponent"))
//		{
//			return Cast<UStaticMeshComponent>(Component);
//		}
//	}
//	return nullptr;
//}

