// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "AssetLoader/RRefillObject.h"
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
	Stacking = 1;

	SpacingStep = 5.0f;
}

// Called when the game starts
void UCPlaceItem::BeginPlay()
{
	Super::BeginPlay();

	if (AssetLoader == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(iTextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Pleas assign a AssetLoader", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}


	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{

		HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

		//HUD->AppendText(FString("Test")); // ID 1
		//HUD->AppendText(FString("Test2")); // ID 2
		//HUD->AppendText(FString("Test3")); // ID 3
		//HUD->AppendText(FString("Test4")); // ID 4
		//HUD->RemoveText(2); // ID 2
		//HUD->ChangeText(1, FString("Blah")); // ID 1

		UInputComponent* PlayerInputComponent = Character->InputComponent;

		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &UCPlaceItem::OnFirePressed);

		PlayerInputComponent->BindAction("IncreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::IncreaseSpacingStep);
		PlayerInputComponent->BindAction("DecreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::DecreaseSpacingStep);

		PlayerInputComponent->BindAction("IncreaseStacking", IE_Pressed, this, &UCPlaceItem::IncreaseStacking);
		PlayerInputComponent->BindAction("DecreaseStacking", IE_Pressed, this, &UCPlaceItem::DecreaseStacking);

		PlayerInputComponent->BindAxis("ChangeSpacing", this, &UCPlaceItem::ChangeSpacing);

		PlayerInputComponent->BindAction("SelectNextItem", IE_Pressed, this, &UCPlaceItem::OnNewItemSelected);
		PlayerInputComponent->BindAction("SelectPreviousItem", IE_Pressed, this, &UCPlaceItem::OnNewItemSelected);

		PlayerInputComponent->BindAction("RotateItem", IE_Pressed, this, &UCPlaceItem::StepRotation);
		PlayerInputComponent->BindAction("TogglePlacingRemovingState", IE_Pressed, this, &UCPlaceItem::TogglePlacingRemovingState);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(iTextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Character is Nullptr. Pausing the game", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}

	// Setup Item
	if (ItemTemplate != nullptr)
	{
		ItemMaterial = GetStaticMesh(ItemTemplate)->GetMaterial(0);
	}

	ItemTag = FName("RefillObject");

	// *** UI Text
	GEngine->AddOnScreenDebugMessage(iTextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(SpacingStep), false);
	GEngine->AddOnScreenDebugMessage(iTextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(iTextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);
}

// Called every frame
void UCPlaceItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if the player stopped selection row or all items on a shelf
	bool bSelectionDeactivated =
		GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftShift) ||
		GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftControl);

	if (bSelectionDeactivated) DeselectItems();

	// Check for any movement or changes in the players input
	CheckPlayerInput();
}

std::pair<FHitResult, TArray<FName>> UCPlaceItem::StartRaytrace()
{
	FVector CamLoc;
	FRotator CamRot;

	Character->Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
	const FVector StartTrace = CamLoc; // trace start is the camera location
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * RaycastRange; // and trace end is the camera location + an offset in the direction

	//Hit contains information about what the raycast hit.
	FHitResult Hit;

	FCollisionQueryParams TraceParams;

	if (CurrentInteractionState == InteractionState::PLACING)
	{
		TraceParams.TraceTag = FName("Shelf"); //  The ray doesn't get blocked by other objects and we're able to place objects behind each other
	}

	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.AddIgnoredActor(ItemTemplate);
	TraceParams.AddIgnoredActors(PhantomItems);

	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldStatic, TraceParams);

	if (bIsDebugMode)
	{
		// DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, true, 3, 0, 1.f);
	}


	if (Hit.GetActor() != nullptr)
	{
		return std::make_pair(Hit, Hit.GetActor()->Tags);
	}
	else
	{
		return std::make_pair(Hit, TArray<FName>());
	}
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

			UStaticMeshComponent* Mesh = GetStaticMesh(elem);

			if (Mesh != nullptr)
			{
				Mesh->SetCollisionProfileName("OverlapOnlyPawn");
				Mesh->bGenerateOverlapEvents = true;
				Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				Mesh->SetSimulatePhysics(true);
				Mesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			}

			PhantomItems.RemoveSwap(elem);
			ObjectPool.RemoveSwap(elem);


		}
	}
}

void UCPlaceItem::RemoveItems()
{
	for (auto & elem : SelectedItems)
	{
		elem->Destroy();
	}

	DeselectItems();
}

void UCPlaceItem::OnFirePressed()
{
	if (FocusedItem == nullptr)
	{
		PlaceItems();
	}
	else
	{
		RemoveItems();
	}
}

void UCPlaceItem::CheckPlayerInput()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	// Check if the player has changed the position
	bPlayerHasMoved = PlayerController->IsInputKeyDown(EKeys::AnyKey) || // Any key doesn't seem to work
		PlayerController->IsInputKeyDown(EKeys::MouseX) ||
		PlayerController->IsInputKeyDown(EKeys::MouseY) ||
		PlayerController->IsInputKeyDown(EKeys::MouseWheelAxis) ||
		PlayerController->IsInputKeyDown(EKeys::W) || // Moved forward
		PlayerController->IsInputKeyDown(EKeys::A) || // Moved to the left
		PlayerController->IsInputKeyDown(EKeys::S) || // Moved backwards
		PlayerController->IsInputKeyDown(EKeys::D) || // Moved to the right
		PlayerController->IsInputKeyDown(EKeys::LeftControl) || // Fill shelf
		PlayerController->IsInputKeyDown(EKeys::LeftShift) || // Row building
		PlayerController->IsInputKeyDown(EKeys::R) || // Rotated item
		PlayerController->IsInputKeyDown(EKeys::SpaceBar); // Spawned new item



	if (bPlayerHasMoved)
	{
		RaytraceResults = StartRaytrace();


		if (CurrentInteractionState == InteractionState::PLACING && RaytraceResults.second.Contains("Shelf"))
		{
			// We hit a shelf
			DisplayPhantomItem(RaytraceResults.first);
			return;
		}
		else
		{
			// Deactivate all former phantom items if we didn't hit a shelf
			for (auto &elem : PhantomItems)
			{
				elem->SetActorHiddenInGame(true);
				elem->SetActorEnableCollision(false);
			}

			PhantomItems.Empty();
		}

		// We hit a refill object
		if (CurrentInteractionState == InteractionState::SELECTING && RaytraceResults.second.Contains(ItemTag))
		{
			bool bIsNewSelection =
				FocusedItem != RaytraceResults.first.GetActor() ||
				GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftShift) ||
				GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftShift) ||
				GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftControl) ||
				GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(EKeys::LeftControl);

			if (bIsNewSelection)
			{
				if (FocusedItem != nullptr)
				{
					GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false); // Deactivate outline of the previous item
					SelectedItems.Remove(FocusedItem); // Remove the old item from the list of selected items
				}

				FocusedItem = RaytraceResults.first.GetActor(); // Assign new item
				GetStaticMesh(FocusedItem)->SetRenderCustomDepth(true); // Reactivate outline effect



				bool bLeftShiftIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftShift);
				bool bLeftControlIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl);

				if (bLeftShiftIsHeldDown)
				{
					DeselectItems(); // Deselect all old items
					SelectRow(); // Select the new row
				}
				else if (bLeftControlIsHeldDown)
				{
					DeselectItems(); // Deselect all old items
					SelectAllItems(); // Select all items on the shelf
				}
				else
				{
					SelectedItems.Add(FocusedItem);
				}
			}
			else
			{
				// Check, if the player pressed the mouse button and delete the objects
				bool bMousePressed = GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::LeftMouseButton);
				if (bMousePressed)
				{
					RemoveItems();
				}
			}

		}
		else
		{
			if (FocusedItem != nullptr)
			{
				GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false);
				SelectedItems.Remove(FocusedItem);
				FocusedItem = nullptr;

				return;
			}
		}
	}
}

void UCPlaceItem::DisplayPhantomItem(FHitResult Hit)
{
	if (Character == nullptr) return;

	ItemTemplate = AssetLoader->GetSpawnedItem();

	if (ItemTemplate == nullptr) return;

	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
	ItemTemplate->K2_SetActorRotation(Rotation, true);// Set  rotation before checking for extends

	// Get position and bounds of the item
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	ItemTemplate->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	ItemTemplate->SetActorRotation(BlockingVol->GetActorRotation() + Rotation); // align rotation to blocking volume + the item rotation

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
	FVector NewPosition = FVector(Position.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z /*- BlockingVolumeExtend.Z*/); // TODO check different items and the extend z value which is commented out right now

	if (LocationToPlaceItem == NewPosition) return; // We didn't actually move the item.

	LocationToPlaceItem = NewPosition;

	// Deactivate all former phantom items
	for (auto &elem : PhantomItems)
	{
		elem->SetActorHiddenInGame(true);
		elem->SetActorEnableCollision(false);
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

		StartVector = GetRowStartPoint(BlockingVol, ItemTemplate, NewPosition);

		if (bIsDebugMode)
		{
			FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis;
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

		//StartVector = BasePointPlane + ItemBoundExtend.X * XAxis + ItemBoundExtend.Y * YAxis + ItemBoundExtend.Z * ZAxis; // TODO check different items and the extend z value which is commented out right now
		StartVector = BasePointPlane + ItemBoundExtend.X * XAxis + ItemBoundExtend.Y * YAxis + (ItemBoundExtend.Z + BlockingVolumeExtend.Z) * ZAxis;

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
	for (size_t s = 0; s < Stacking; s++)
	{
		for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
		{
			for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
			{
				// Create clone of the item to display
				AActor* Clone = GetCloneActor(ItemTemplate);

				if (Clone == nullptr) return;

				PhantomItems.Add(Clone);

				// The points within the blocking volume
				FVector RelativePointOnXAxis = SpacingX * XAxis * 2 * ItemBoundExtend.X * i;
				FVector RelativePointOnYAxis = SpacingY * YAxis * 2 * ItemBoundExtend.Y * j;
				FVector RelativePositionOnZAxis = ZAxis * 2 * ItemBoundExtend.Z * s;

				NewPosition = StartVector + RelativePointOnXAxis + RelativePointOnYAxis + RelativePositionOnZAxis;

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

void UCPlaceItem::SelectRow()
{
	ABlockingVolume* BlockingVol = FindShelf(FocusedItem);

	if (BlockingVol == nullptr)
	{
		if (SelectedItems.Contains(FocusedItem) == false) {
			SelectedItems.Add(FocusedItem);
		}

		return; // That wasn't a blocking volume we hit
	}

	TArray<FHitResult> Hits;

	FVector StartVectorTemp = GetRowStartPoint(BlockingVol, FocusedItem, FocusedItem->GetActorLocation());
	FVector StartVector = FVector(
		StartVectorTemp.X,
		StartVectorTemp.Y,
		BlockingVol->GetActorLocation().Z + 2 * BlockingVol->Brush->Bounds.BoxExtent.Z
	);

	FVector EndVector = StartVector + 2 * BlockingVol->Brush->Bounds.BoxExtent.X * BlockingVol->GetActorForwardVector();

	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = FName(ItemTag); //  Only hit Refill Items

	GetWorld()->LineTraceMultiByChannel(Hits, StartVector, EndVector, ECollisionChannel::ECC_Pawn, TraceParams);

	for (auto& elem : Hits)
	{
		AActor* HitActor = elem.GetActor();
		if (HitActor != nullptr && HitActor->Tags.Contains(ItemTag))
		{
			UStaticMeshComponent* MeshComp = GetStaticMesh(HitActor);
			if (MeshComp != nullptr)
			{
				MeshComp->SetRenderCustomDepth(true);
			}

			CheckStackedNeighbour(HitActor);
			SelectedItems.Add(HitActor);
		}
	}


	if (bIsDebugMode)
	{
		DrawDebugLine(GetWorld(), StartVector, EndVector, FColor::Blue, false, 3, 5);
	}

}

void UCPlaceItem::SelectAllItems()
{
	ABlockingVolume* BlockingVol = FindShelf(FocusedItem);

	if (BlockingVol == nullptr)
	{
		if (SelectedItems.Contains(FocusedItem) == false)
		{
			SelectedItems.Add(FocusedItem);
		}

		return; // That wasn't a blocking volume we hit
	}

	TArray<AActor*> ItemsOnShelf;
	BlockingVol->GetOverlappingActors(ItemsOnShelf);

	for (auto & HitActor : ItemsOnShelf)
	{
		if (HitActor->Tags.Contains(ItemTag))
		{
			UStaticMeshComponent* MeshComp = GetStaticMesh(HitActor);
			if (MeshComp != nullptr)
			{
				MeshComp->SetRenderCustomDepth(true);
			}

			CheckStackedNeighbour(HitActor);
			SelectedItems.Add(HitActor);
		}
	}
}

void UCPlaceItem::DeselectItems()
{
	for (auto& elem : SelectedItems)
	{
		GetStaticMesh(elem)->SetRenderCustomDepth(false);
	}

	SelectedItems.Empty();


	// Re-enable outline for focusedItem if there is any, which means the player is still pointing at an item
	bool bLeftShiftIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftShift);
	bool bLeftControlIsHeldDown = GetWorld()->GetFirstPlayerController()->IsInputKeyDown(EKeys::LeftControl);

	if (FocusedItem != nullptr && bLeftShiftIsHeldDown == false && bLeftControlIsHeldDown == false)
	{
		GetStaticMesh(FocusedItem)->SetRenderCustomDepth(true);
		SelectedItems.Add(FocusedItem);
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

void UCPlaceItem::IncreaseStacking()
{
	Stacking++;
}

void UCPlaceItem::DecreaseStacking()
{
	Stacking--;
	if (Stacking < 1) Stacking = 1;
}

void UCPlaceItem::StepRotation()
{
	Rotation += FRotator(0, 90, 0);
}

void UCPlaceItem::TogglePlacingRemovingState()
{
	if (CurrentInteractionState == InteractionState::PLACING)
	{
		CurrentInteractionState = InteractionState::SELECTING;
	}
	else
	{
		CurrentInteractionState = InteractionState::PLACING;
	}
}

AActor * UCPlaceItem::GetCloneActor(AActor * ActorToClone)
{

	for (auto& object : ObjectPool)
	{
		if (object->bHidden)
		{
			object->SetActorHiddenInGame(false);
			object->SetActorEnableCollision(true);

			UStaticMeshComponent* MeshComponent = GetStaticMesh(object);
			UStaticMesh* MeshOfActorToClone = GetStaticMesh(ActorToClone)->GetStaticMesh();

			if (MeshComponent->GetStaticMesh() != MeshOfActorToClone)
			{
				MeshComponent->SetStaticMesh(MeshOfActorToClone);
			}

			if (MeshComponent->GetMaterial(0) != RedMaterial)
			{ // We don't want to apply the red material
				ItemMaterial = MeshComponent->GetMaterial(0);
			}

			MeshComponent->SetCollisionProfileName("OverlapOnlyPawn");
			MeshComponent->SetSimulatePhysics(false);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

			// UE_LOG(LogTemp, Warning, TEXT("Reused object"));

			object->K2_SetActorRotation(ItemTemplate->GetActorRotation(), false);

			return object;
		}
	}

	// If we came here, we don't have an unused object. We create a new one
	FActorSpawnParameters Parameters;
	Parameters.Template = ActorToClone;
	AActor* Clone = GetWorld()->SpawnActor<ARRefillObject>(ARRefillObject::StaticClass(), Parameters);

	UStaticMeshComponent* MeshComponent = GetStaticMesh(Clone);

	ItemMaterial = MeshComponent->GetMaterial(0);

	MeshComponent->SetCollisionProfileName("NoCollision");

	//MeshComponent->SetCollisionProfileName("OverlapOnlyPawn");	
	//MeshComponent->bGenerateOverlapEvents = true;
	//MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//MeshComponent->SetSimulatePhysics(false);
	//MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

	ObjectPool.Add(Clone);

	if (Clone->Tags.Contains(ItemTag) == false)
	{
		Clone->Tags.Add(ItemTag);
	}

	return Clone;
}


bool UCPlaceItem::CheckCollisions(AActor * Actor)
{
	
	// *** Check for collisions
	FHitResult OutHitRes;
	Actor->AddActorWorldOffset(FVector(0.f, 0.f, 0.05f), true, &OutHitRes);


	if (OutHitRes.GetActor() != nullptr && OutHitRes.GetActor()->Tags.Contains("Shelf") == false
		&& PhantomItems.Contains(OutHitRes.GetActor()) == false)
	{

		// There is a collision with another item
		UE_LOG(LogTemp, Warning, TEXT("Colliding with %s"), *OutHitRes.GetActor()->GetName());

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

		Actor->AddActorWorldOffset(FVector(0.f, 0.f, -0.1f), true, &OutHitRes);

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

void UCPlaceItem::OnNewItemSelected()
{
	// Draw Name of selected item to HUD
	/*if (ItemTemplate != nullptr)
	{*/
	HUD->ShowRefillItem(FPaths::GameContentDir().Append("Cache/test.png"), FString("Test"));
	//}
}

FVector UCPlaceItem::GetRowStartPoint(ABlockingVolume* BlockingVolume, AActor* ItemToStartFrom, FVector ImpactPoint)
{
	// Calculate an intersection point between a line from the impact point to the lower (relative) x axis of the blocking volume
	FVector BlockingVolumeExtend = BlockingVolume->Brush->Bounds.BoxExtent;
	FVector XAxis = BlockingVolume->GetActorForwardVector();
	FVector YAxis = BlockingVolume->GetActorRightVector();
	FVector BlockingVolumeOrigin = BlockingVolume->GetActorLocation();

	FVector ItemOrigin;
	FVector ItemBoundExtend;
	ItemToStartFrom->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	FVector SecondPointOfLine = ImpactPoint - BlockingVolumeExtend.X * XAxis;
	FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis;
	FVector NormalVectorOfPlane = XAxis;

	FVector IntersectionLinePlane = FMath::LinePlaneIntersection(ImpactPoint, SecondPointOfLine, BasePointPlane, NormalVectorOfPlane);

	FVector StartVector = IntersectionLinePlane + ItemBoundExtend.X * XAxis;
	return StartVector;
}

void UCPlaceItem::CheckStackedNeighbour(AActor * FromActor)
{
	// *** Raycast upwards to get stacked items ***
	FVector HitActorOrigin;
	FVector HitActorExtend;
	FromActor->GetActorBounds(false, HitActorOrigin, HitActorExtend);

	TArray<FHitResult> StackHits;
	FCollisionQueryParams TraceParamsStack;

	FVector StackStartVector = HitActorOrigin;
	FVector StackEndVector = StackStartVector + FromActor->GetActorUpVector() * HitActorExtend * 2;

	GetWorld()->LineTraceMultiByChannel(StackHits, StackStartVector, StackEndVector, ECollisionChannel::ECC_Pawn, TraceParamsStack);

	for (const auto& elem : StackHits)
	{
		AActor* StackedActor = elem.GetActor();
		if (StackedActor != nullptr && StackedActor->Tags.Contains(ItemTag))
		{
			UStaticMeshComponent* MeshCompStackItem = GetStaticMesh(StackedActor);
			if (MeshCompStackItem != nullptr)
			{
				MeshCompStackItem->SetRenderCustomDepth(true);
			}

			if (SelectedItems.Contains(StackedActor) == false)
			{
				SelectedItems.Add(StackedActor);
				CheckStackedNeighbour(StackedActor);
			}
		}
	}
}

ABlockingVolume * UCPlaceItem::FindShelf(AActor * FromActor)
{
	ABlockingVolume* BlockingVol = nullptr;

	TArray<FHitResult> BlockingVolumeHits;
	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = FName("Shelf");
	TraceParams.AddIgnoredActor(FocusedItem);

	FVector StartVector = FocusedItem->GetActorLocation();
	FVector EndVector = StartVector - FocusedItem->GetActorUpVector() * 100.0f; // Very long vektor

	GetWorld()->LineTraceMultiByChannel(BlockingVolumeHits, StartVector, EndVector, ECollisionChannel::ECC_Camera, TraceParams);

	for (auto & elem : BlockingVolumeHits)
	{
		AActor* HitActor = elem.GetActor();

		if (HitActor != nullptr && HitActor->Tags.Contains("Shelf"))
		{
			BlockingVol = Cast<ABlockingVolume>(HitActor);
			break;
		}
	}

	return BlockingVol;
}


