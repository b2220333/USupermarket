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
	SpacingX = 1;
	SpacingY = 1;

	SpacingStep = 5.0f;
}

// Called when the game starts
void UCPlaceItem::BeginPlay()
{
	Super::BeginPlay();



	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{

		AMyHUD* HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

		//HUD->AppendText(FString("Test")); // ID 1
		//HUD->AppendText(FString("Test2")); // ID 2
		//HUD->AppendText(FString("Test3")); // ID 3
		//HUD->AppendText(FString("Test4")); // ID 4
		//HUD->RemoveText(2); // ID 2
		//HUD->ChangeText(1, FString("Blah")); // ID 1

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
	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(SpacingStep), false);
	GEngine->AddOnScreenDebugMessage(iTextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(iTextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);
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
		SpacingX += Val / SpacingStep;

		if (SpacingX < 1) SpacingX = 1;

		LocationToPlaceItem = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	if (bLeftControlIsHeldDown)
	{
		SpacingY += Val / SpacingStep;

		if (SpacingY < 1) SpacingY = 1;

		LocationToPlaceItem = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	GEngine->AddOnScreenDebugMessage(iTextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(iTextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);

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
	Item->SetActorRotation(FQuat(0, 0, 0, 0)); // Set zero rotation before checking for extends

	// Get position and bounds of the item
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	Item->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	Item->SetActorRotation(BlockingVol->GetActorRotation()); // align rotation to blocking volume to measure the original extends
	
	FVector BlockingVolumeOrigin;
	FVector NotUsed;

	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, NotUsed);

	// Use the extends of the brush
	FVector BlockingVolumeExtend = BlockingVol->Brush->Bounds.BoxExtent;

	// Get direction of x, y and z axis
	FVector XAxis = BlockingVol->GetActorForwardVector();
	FVector YAxis = BlockingVol->GetActorRightVector();
	FVector ZAxis = BlockingVol->GetActorUpVector();

	// Calculating new position
	FVector Position = Hit.ImpactPoint;
	FVector NewPosition = FVector(Position.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z);

	// Check if we really moved the item this frame
	if (NewPosition.Equals(LocationToPlaceItem) == false)
	{
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
			// Placing a row

			// Calculate how many items we can place in a row
			AmountOfItemsPossibleX = FMath::FloorToInt((2 * BlockingVolumeExtend.X) / (2 * ItemBoundExtend.X * SpacingX));

			//UE_LOG(LogTemp, Warning, TEXT("Block Extend %f"), BlockingVolumeExtend.X);
			//UE_LOG(LogTemp, Warning, TEXT("Item Extend %f"), ItemBoundExtend.X);
			//UE_LOG(LogTemp, Warning, TEXT("Amount %i"), AmountOfItemsPossibleX);

			// Calculate an intersection point between a line from the impact point to the lower (relative) x axis of the blocking volume
			// to determine where to start the row
			FVector RaycastImpactPoint = NewPosition;
			FVector SecondPointOfLine = RaycastImpactPoint - BlockingVolumeExtend.X * XAxis;
			FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis;
			FVector NormalVectorOfPlane = XAxis;

			FVector IntersectionLinePlane = FMath::LinePlaneIntersection(RaycastImpactPoint, SecondPointOfLine, BasePointPlane, NormalVectorOfPlane);

			StartVector = IntersectionLinePlane + ItemBoundExtend.X * XAxis; // Move the start point more inside the blocking volume depending on the extend of the item

			if (bIsDebugMode)
			{
				DrawDebugPoint(GetWorld(), StartVector, 5, FColor::Blue, false, 3);
				DrawDebugLine(GetWorld(), BasePointPlane, BasePointPlane + BlockingVolumeExtend.Y * YAxis * 2, FColor::Blue, false, 3, 5);
			}
		}
		else if (bLeftControlIsHeldDown)
		{
			// Fill with items

			// Calculate the amount of items we can place on the shelf in x and y axis (column and row)
			AmountOfItemsPossibleX = FMath::FloorToInt(BlockingVolumeExtend.X / (ItemBoundExtend.X * SpacingX));
			AmountOfItemsPossibleY = FMath::FloorToInt(BlockingVolumeExtend.Y / (ItemBoundExtend.Y * SpacingY));

			// The lower left corner of the blocking volume
			FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis - BlockingVolumeExtend.Z * ZAxis;
			
			StartVector = BasePointPlane + ItemBoundExtend.X * XAxis + ItemBoundExtend.Y * YAxis + ItemBoundExtend.Z * ZAxis;

			if (bIsDebugMode)
			{
				DrawDebugPoint(GetWorld(), StartVector, 5, FColor::Blue, false, 3);
				DrawDebugLine(GetWorld(), BasePointPlane, BasePointPlane + BlockingVolumeExtend.Y * YAxis * 2, FColor::Blue, false, 3, 5);
				DrawDebugLine(GetWorld(), BasePointPlane, BasePointPlane + BlockingVolumeExtend.X * XAxis * 2, FColor::Blue, false, 3, 5);
			}
		}
		else
		{
			StartVector = NewPosition;
		}

		// Now place the items
		for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
		{
			for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
			{
				// Create clone of the item to display
				AActor* Clone = GetCloneActor(Item);

				if (Clone == nullptr) return;

				PhantomItems.Add(Clone);

				// The points within the blocking volume
				FVector RelativePointOnXAxis = SpacingX * XAxis * 2 * ItemBoundExtend.X * i ;
				FVector RelativePointOnYAxis = SpacingY * YAxis * 2 * ItemBoundExtend.Y * j;

				NewPosition = StartVector + RelativePointOnXAxis + RelativePointOnYAxis;

				if (bIsDebugMode)
				{
					DrawDebugPoint(GetWorld(), NewPosition, 10, FColor::Green, false, 3);
				}

				Clone->SetActorLocation(NewPosition);

				bItemCanBePlaced = CheckCollisions(Clone);
				if (bItemCanBePlaced == false) return;
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
	SpacingStep *= 2;
	if (SpacingStep <= 0) SpacingStep = 0.1f;
	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / SpacingStep), false);
}

void UCPlaceItem::DecreaseSpacingStep()
{
	SpacingStep /= 2;

	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / SpacingStep), false);
}

AActor * UCPlaceItem::GetCloneActor(AActor * ActorToClone)
{

	// TODO Use object pool
	FActorSpawnParameters Parameters;
	Parameters.Template = ActorToClone;
	AActor* Clone = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Parameters);
	return Clone;
}

bool UCPlaceItem::CheckCollisions(AActor * Actor)
{
	// *** Check for collisions
	FHitResult OutHitRes;
	Actor->AddActorWorldOffset(FVector(0.f, 0.f, .1f), true, &OutHitRes);

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

		// We need to return here, otherwise another item in the row which is not colliding would turn
		// the 'bItemCanBePlaced' to true. Only the items before the collision will be shown. If this is not 
		// what we want, we also can do something like bItemCanBePlaced = bItemCanBePlaced && false and do not return here
		return false;
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
		return true;
	}
}


