// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "MyHUD.h"
#include "CPlaceItem.h"


// Sets default values for this component's properties
UCPlaceItem::UCPlaceItem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	RaycastRange = 200;
	fSpacingX = 0;
	fSpacingY = 0;

	fSpacingStep = 1;
}

// Called when the game starts
void UCPlaceItem::BeginPlay()
{
	Super::BeginPlay();



	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{

		AMyHUD* HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

		HUD->AppendText(FString("Test")); // ID 1
		HUD->AppendText(FString("Test2")); // ID 2
		HUD->AppendText(FString("Test3")); // ID 3
		HUD->AppendText(FString("Test4")); // ID 4
		HUD->RemoveText(2); // ID 2
		HUD->ChangeText(1, FString("Blah")); // ID 1

		UInputComponent* PlayerInputComponent = Character->InputComponent;

		//PlayerInputComponent->BindAction("PlaceSingeItem", IE_Pressed, this, &UCPlaceItem::PlaceSingleItem);
		//PlayerInputComponent->BindAction("PlaceRowOfItems", IE_Pressed, this, &UCPlaceItem::PlaceRowItems);
		//PlayerInputComponent->BindAction("FillWithItems", IE_Pressed, this, &UCPlaceItem::FillItems);

		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &UCPlaceItem::PlaceItems);

		PlayerInputComponent->BindAction("IncreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::IncreaseSpacingStep);
		PlayerInputComponent->BindAction("DecreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::DecreaseSpacingStep);

		PlayerInputComponent->BindAxis("ChangeSpacing", this, &UCPlaceItem::ChangeSpacing);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(iTextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Character is Nullptr. Pausing the game", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}

	// Setup Item
	if (Item != nullptr)
	{
		ItemMaterial = GetStaticMesh(Item)->GetMaterial(0);
		GetStaticMesh(Item)->SetCollisionProfileName("OverlapOnlyPawn");
	}

	// *** UI Text
	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(fSpacingStep), false);
	GEngine->AddOnScreenDebugMessage(iTextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(fSpacingX), false);
	GEngine->AddOnScreenDebugMessage(iTextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(fSpacingY), false);
}

// Called every frame
void UCPlaceItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	bool bPlayerHasMoved = PlayerController->IsInputKeyDown(EKeys::MouseX) || PlayerController->IsInputKeyDown(EKeys::MouseY) || PlayerController->IsInputKeyDown(EKeys::MouseWheelAxis) || PlayerController->IsInputKeyDown(EKeys::W) || PlayerController->IsInputKeyDown(EKeys::A) || PlayerController->IsInputKeyDown(EKeys::S) || PlayerController->IsInputKeyDown(EKeys::D) || PlayerController->IsInputKeyDown(EKeys::LeftControl) || PlayerController->IsInputKeyDown(EKeys::LeftShift);

	if (bPlayerHasMoved)
	{
		RaytraceResults = StartRaytrace();

		if (RaytraceResults.second == true && Item != nullptr)
		{
			DisplayPhantomItem(RaytraceResults.first);
		}
		else
		{
			for (auto &elem : PhantomItems)
			{
				elem->Destroy();
			}
			PhantomItems.Empty();
		}
	}
}

std::pair<FHitResult, bool> UCPlaceItem::StartRaytrace()
{
	FVector CamLoc;
	FRotator CamRot;

	Character->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * RaycastRange; // and trace end is the camera location + an offset in the direction

	//Collision parameters. The following syntax means that we don't want the trace to be complex
	FCollisionQueryParams CollisionParameters;

	//Hit contains information about what the raycast hit.
	FHitResult Hit;


	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = FName("Shelf");
	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.AddIgnoredActor(Item);


	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_Camera, TraceParams);

	// DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true, 3, 0, 1.f);


	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
	bool bIsBlockingVolume = false;

	if (BlockingVol != nullptr)
	{
		for (FName tag : BlockingVol->Tags)
		{
			if (tag.IsEqual("Shelf"))
			{ // Todo Hardcoded name
				bIsBlockingVolume = true;
				break;
			}
		}
	}

	return std::make_pair(Hit, bIsBlockingVolume);
}

void UCPlaceItem::ChangeSpacing(const float Val)
{
	if (Val == 0) return;

	bool bLeftShiftIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftShift);
	bool bLeftControlIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl);

	if (bLeftShiftIsHeldDown)
	{
		fSpacingX += Val / fSpacingStep;

		if (fSpacingX < 0) fSpacingX = 0;

		LocationToPlaceItem = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	if (bLeftControlIsHeldDown)
	{
		fSpacingY += Val / fSpacingStep;

		if (fSpacingY < 0) fSpacingY = 0;

		LocationToPlaceItem = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	GEngine->AddOnScreenDebugMessage(iTextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(fSpacingX), false);
	GEngine->AddOnScreenDebugMessage(iTextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(fSpacingY), false);

}

void UCPlaceItem::PlaceItems()
{
	if (bItemCanBePlaced)
	{
		// We only need to remove the items from the phantom item array so they won't get destroyed the next time we move the mouse
		TArray<AActor*> ItemsToPlace = PhantomItems;

		for (auto &elem : ItemsToPlace)
		{
			PhantomItems.RemoveSwap(elem);
		}
	}
}

void UCPlaceItem::DisplayPhantomItem(FHitResult Hit)
{
	if (Character == nullptr) return;

	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());

	// Get position and bounds of the item
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	// Get Position and bound of the blocking volume
	FVector BlockingVolumeOrigin;
	FVector BlockingVolumeExtend;
	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);

	// Calculating new position
	FVector Position = Hit.ImpactPoint;

	FVector NewPosition = FVector(Position.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);

	// Check if we really moved the item this frame
	if (NewPosition.Equals(LocationToPlaceItem) == false)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Old Position %s New Position %s"), *LocationToPlaceItem.ToCompactString(), *NewPosition.ToCompactString());
		LocationToPlaceItem = NewPosition;

		// Delete all former phantom items
		for (auto &elem : PhantomItems)
		{
			elem->Destroy(); // TODO Destroying objects might be a little slow. Consider using an object pool
		}

		PhantomItems.Empty();

		int AmountOfItemsPossibleX = 1;
		int AmountOfItemsPossibleY = 1;

		bool bLeftShiftIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftShift); // Placing a row
		bool bLeftControlIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl); // Fill with items

		FVector StartVector;
		if (bLeftShiftIsHeldDown)
		{
			AmountOfItemsPossibleX = FMath::FloorToInt(BlockingVolumeExtend.X / (ItemBoundExtend.X + fSpacingX / 2));

			StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);
		}
		else if (bLeftControlIsHeldDown)
		{
			AmountOfItemsPossibleX = FMath::FloorToInt(BlockingVolumeExtend.X / (ItemBoundExtend.X + fSpacingX / 2));
			AmountOfItemsPossibleY = FMath::FloorToInt(BlockingVolumeExtend.Y / (ItemBoundExtend.Y + fSpacingY / 2));

			StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, BlockingVolumeOrigin.Y - BlockingVolumeExtend.Y + ItemBoundExtend.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);
		}
		else
		{
			StartVector = NewPosition;
		}

		for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
		{
			for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
			{
				// *** Create clones of the item to display
				FActorSpawnParameters Parameters;
				Parameters.Template = Item;
				AActor* Clone = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);
				if (Clone == nullptr) return;

				PhantomItems.Add(Clone);
				// ***

				float BlockingVolumeRotation = BlockingVol->GetActorRotation().GetDenormalized().Yaw;

				NewPosition = FVector(StartVector.X + i * (ItemBoundExtend.X * 2 + fSpacingX), StartVector.Y + j * (ItemBoundExtend.Y * 2 + fSpacingY), StartVector.Z);



				if (BlockingVolumeRotation <= 45 || BlockingVolumeRotation >= 315)
				{
					// Blocking volume is mostly rotated torwards world rotation the world 
					// UE_LOG(LogTemp, Warning, TEXT("Rotation is 12 o'clock"));
					NewPosition = FVector(StartVector.X + i * (ItemBoundExtend.X * 2 + fSpacingX), StartVector.Y + j * (ItemBoundExtend.Y * 2 + fSpacingY), StartVector.Z);
				}
				else if (225 <= BlockingVolumeRotation && BlockingVolumeRotation <= 315)
				{
					// Blocking volume is mostly rotated 90 degree
				    // UE_LOG(LogTemp, Warning, TEXT("Rotation is 9 o'clock"));

				}
				else if (135 <= BlockingVolumeRotation && BlockingVolumeRotation <= 225)
				{
					// Blocking volume is mostly rotated 180 degree
					// UE_LOG(LogTemp, Warning, TEXT("Rotation is 6 o'clock"));
				//	float NewX = StartVector.X + BlockingVolumeExtend.X - 0 * ItemBoundExtend.X * 2;

			//		NewPosition = FVector(NewX, StartVector.Y + j * (ItemBoundExtend.Y * 2 + fSpacingY), StartVector.Z);
					// StartVector.X - i * (ItemBoundExtend.X * 2 + fSpacingX)
				}
				else if (45 <= BlockingVolumeRotation && BlockingVolumeRotation <= 135)
				{
					// Blocking volume is mostly rotated 270 degree
				    // UE_LOG(LogTemp, Warning, TEXT("Rotation is 3 o'clock"));
				}


				Clone->SetActorLocation(NewPosition);

				// *** Check for collisions
				FHitResult OutHitRes;
				Clone->AddActorWorldOffset(FVector(0.f, 0.f, .1f), true, &OutHitRes);

				if (OutHitRes.GetActor() != nullptr && OutHitRes.GetActor()->Tags.Contains("Shelf") == false
					&& PhantomItems.Contains(OutHitRes.GetActor()) == false)
				{

					// There is a collision with another item
					// UE_LOG(LogTemp, Warning, TEXT("Overlaping with %s"), *OutHitRes.GetActor()->GetName());

					if (RedMaterial != nullptr)
					{
						for (auto &elem : PhantomItems)
						{
							// Color it red
							UStaticMeshComponent* Mesh = GetStaticMesh(elem);

							if (Mesh != nullptr)
							{
								Mesh->SetMaterial(0, RedMaterial);
							}
						}
					}

					bItemCanBePlaced = false;

					// We need to return here, otherwise another item in the row which is not colliding would turn
					// the 'bItemCanBePlaced' to true. Only the items before the collision will be shown. If this is not 
					// what we want, we also can do something like bItemCanBePlaced = bItemCanBePlaced && false and do not return here
					return;
				}
				else
				{
					if (ItemMaterial != nullptr)
					{
						for (auto &elem : PhantomItems)
						{
							// Color it normal
							UStaticMeshComponent* Mesh = GetStaticMesh(elem);

							if (Mesh != nullptr)
							{
								Mesh->SetMaterial(0, ItemMaterial);
							}

						}
					}

					bItemCanBePlaced = true;
				}
			}
		}
	}
}

UStaticMeshComponent* UCPlaceItem::GetStaticMesh(AActor* Actor)
{
	for (auto Component : Actor->GetComponents())
	{
		if (Component->GetName().Contains("StaticMeshComponent"))
		{
			return Cast<UStaticMeshComponent>(Component);
		}
	}
	return nullptr;
}

void UCPlaceItem::IncreaseSpacingStep()
{
	fSpacingStep *= 2;
	if (fSpacingStep <= 0) fSpacingStep = 0.1f;
	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / fSpacingStep), false);
}

void UCPlaceItem::DecreaseSpacingStep()
{
	fSpacingStep /= 2;

	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / fSpacingStep), false);
}





// Deprecated
void UCPlaceItem::PlaceItemAtPosition(FVector Position) {

	//if (bItemCanBePlaced == false) return;

	//FActorSpawnParameters Parameters;
	//Parameters.Template = Item;
	//AActor* Clone = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);
	//if (Clone == nullptr) return;

	//Clone->SetActorLocation(Position);
}

// Deprecated
void UCPlaceItem::PlaceSingleItem()
{
	// PlaceItemAtPosition(LocationToPlaceItem);
	PlaceItems();
}

// Deprecated
void UCPlaceItem::PlaceRowItems()
{
	PlaceItems();

	//// *** Raytracing *** 
	//// std::pair<FHitResult, bool> RaytraceResults = StartRaytrace();
	//if (RaytraceResults.second == false) return; // We didn't hit a blocking volume

	//FHitResult Hit = RaytraceResults.first;
	//ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());

	//// Get position and bounds of the item
	//FVector ItemOrigin;
	//FVector ItemBoundExtend;
	//Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	//// Get Position and bound of the blocking volume
	//FVector BlockingVolumeOrigin;
	//FVector BlockingVolumeExtend;
	//BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);

	//// Calculating new positions
	//FVector Position = Hit.ImpactPoint;

	//int AmountOfItemsPossible = BlockingVolumeExtend.X / (ItemBoundExtend.X + fSpacingX / 2);

	//UE_LOG(LogTemp, Warning, TEXT("Placing %i items"), AmountOfItemsPossible);

	////FVector StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, Position.Y, Position.Z + ItemBoundExtend.Z - (BlockingVolumeExtend.Z * 2));
	//FVector StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);

	//for (size_t i = 0; i < AmountOfItemsPossible; i++)
	//{
	//	FVector NewPosition = FVector(StartVector.X + i * (ItemBoundExtend.X * 2 + fSpacingX), StartVector.Y, StartVector.Z);
	//	PlaceItemAtPosition(NewPosition);
	//}
}

// Deprecated
void UCPlaceItem::FillItems()
{
	PlaceItems();

	//// *** Raytracing *** 
	//// std::pair<FHitResult, bool> RaytraceResults = StartRaytrace();
	//if (RaytraceResults.second == false) return; // We didn't hit a blocking volume

	//FHitResult Hit = RaytraceResults.first;
	//ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());

	//// Get position and bounds of the item
	//FVector ItemOrigin;
	//FVector ItemBoundExtend;
	//Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	//// Get Position and bound of the blocking volume
	//FVector BlockingVolumeOrigin;
	//FVector BlockingVolumeExtend;
	//BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, BlockingVolumeExtend);

	//FVector Position = Hit.ImpactPoint;

	//int AmountOfItemsPossibleX = BlockingVolumeExtend.X / ItemBoundExtend.X;
	//int AmountOfItemsPossibleY = BlockingVolumeExtend.Y / ItemBoundExtend.Y;

	//UE_LOG(LogTemp, Warning, TEXT("Placing %i items"), AmountOfItemsPossibleX * AmountOfItemsPossibleY);

	//FVector StartVector = FVector(BlockingVolumeOrigin.X - BlockingVolumeExtend.X + ItemBoundExtend.X, BlockingVolumeOrigin.Y - BlockingVolumeExtend.Y + ItemBoundExtend.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);

	//for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
	//{
	//	for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
	//	{
	//		FVector NewPosition = FVector(StartVector.X + i * ItemBoundExtend.X * 2, StartVector.Y + j * ItemBoundExtend.Y * 2, StartVector.Z);
	//		PlaceItemAtPosition(NewPosition);
	//	}
	//}
}


