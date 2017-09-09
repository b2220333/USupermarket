// Fill out your copyright notice in the Description page of Project Settings.
#define TAG_KEY_SHELF "Shelf"
#define TAG_SHELF "Refill;Shelf,True;"

#define TAG_KEY_HOOK "Hook"
#define TAG_HOOK "Refill;Hook,True;"

#define TAG_KEY_ITEM "RefillObject"
#define TAG_ITEM "Refill;RefillObject,True;"

#define MINIMUM_GAP_BETWEEN_ITEMS 0.1f // The gap between each item when playcing them in a row or when filling the shelf

#include "Refill.h"
#include "../AssetLoader/RRefillObject.h"
#include "TagStatics.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CPlaceItem.h"


UCPlaceItem::UCPlaceItem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	RaycastRange = 200;
	SpacingX = 1;
	SpacingY = 1;
	Stacking = 1;

	SpacingStep = 1.0f;
}

void UCPlaceItem::BeginPlay()
{
	Super::BeginPlay();

	// Get all actors with specific tag
	ListOfRefillTitemsPlacedInWorld = FTagStatics::GetActorsWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_ITEM, "True");
	SetOfShelves = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_SHELF, "True");
	//SetOfHooks = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_HOOK, "True");
	SetOfHooks = FTagStatics::GetComponentSetWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_HOOK, "True");

	// Read all the Hook names
	for (auto& HookComponent : SetOfHooks) {
		HooknamesToHookComponent.Add(HookComponent->GetFName(), HookComponent);
	}

	UE_LOG(LogTemp, Log, TEXT("%s: Items found %i"), *FString(__FUNCTION__), ListOfRefillTitemsPlacedInWorld.Num());
	UE_LOG(LogTemp, Log, TEXT("%s: Shelves found %i"), *FString(__FUNCTION__), SetOfShelves.Num());
	UE_LOG(LogTemp, Log, TEXT("%s: Hooks found %i"), *FString(__FUNCTION__), SetOfHooks.Num());
	// *********************************************** 

	if (AssetLoader == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(TextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Please assign an AssetLoader actor", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}
	else {
		// Bind function to delegate
		UE_LOG(LogTemp, Log, TEXT("%s: Bind OnNewItemSpawned"), *FString(__FUNCTION__));
		AssetLoader->OnItemSpawend.AddDynamic(this, &UCPlaceItem::OnNewItemSpawned);
	}

	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(TextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Character is Nullptr. Pausing the game", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}

	// *** UI Text
	GEngine->AddOnScreenDebugMessage(TextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing Steps: x" + FString::SanitizeFloat(SpacingStep), false);
	GEngine->AddOnScreenDebugMessage(TextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(TextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);
	GEngine->AddOnScreenDebugMessage(TextID_PlacingSelectingMode, 1000000.0f, FColor::Blue, "Current Mode: PLACING", false);
}

void UCPlaceItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check for any movement or changes in the players input
	CheckPlayerInput();

	if (bLeftShiftIsHeldDown) {
		GEngine->AddOnScreenDebugMessage(TextID_RowMode, 0.1f, FColor::Green, "Row-Mode", false);
	}
	else if (bLeftControlIsHeldDown) {

		if (bLeftControlIsHeldDown) GEngine->AddOnScreenDebugMessage(TextID_WholeShelfMode, 0.1f, FColor::Green, "WholeShelf-Mode", false);
	}
	ResetInputKeys();
}

void UCPlaceItem::SetupKeyBindings(UInputComponent * PlayerInputComponent)
{
	// ********** Setup bindings
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &UCPlaceItem::OnFirePressed);

	PlayerInputComponent->BindAction("IncreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::IncreaseSpacingStep);
	PlayerInputComponent->BindAction("DecreaseSpacingStep", IE_Pressed, this, &UCPlaceItem::DecreaseSpacingStep);

	PlayerInputComponent->BindAction("IncreaseStacking", IE_Pressed, this, &UCPlaceItem::IncreaseStacking);
	PlayerInputComponent->BindAction("DecreaseStacking", IE_Pressed, this, &UCPlaceItem::DecreaseStacking);

	PlayerInputComponent->BindAxis("ChangeSpacing", this, &UCPlaceItem::ChangeSpacing);

	PlayerInputComponent->BindAction("RotateItem", IE_Pressed, this, &UCPlaceItem::StepRotation);
	PlayerInputComponent->BindAction("TogglePlacingRemovingState", IE_Pressed, this, &UCPlaceItem::TogglePlacingRemovingState);

	PlayerInputComponent->BindAction("RowMode", IE_Pressed, this, &UCPlaceItem::OnKeyRowModePressed);
	PlayerInputComponent->BindAction("RowMode", IE_Released, this, &UCPlaceItem::OnKeyRowModeReleased);

	PlayerInputComponent->BindAction("WholeShelfMode", IE_Pressed, this, &UCPlaceItem::OnKeyWholeShelfModePressed);
	PlayerInputComponent->BindAction("WholeShelfMode", IE_Released, this, &UCPlaceItem::OnKeyWholeShelfModeReleased);
	// ***************************
}

FHitResult UCPlaceItem::StartRaytrace()
{
	// Camera location and rotation
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
		TraceParams.AddIgnoredActors(ListOfRefillTitemsPlacedInWorld); // Ignore all placed RefillITems in the world so we can place new items behind them
	}

	TraceParams.AddIgnoredActor(GetOwner()); // Ignore the player
	TraceParams.AddIgnoredActor(ItemTemplate); // Ignore the item in our hand
	TraceParams.AddIgnoredActors(PhantomItems); // Ignore al phantom items we are currently showing

	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldStatic, TraceParams);

	return Hit;
}

void UCPlaceItem::ChangeSpacing(const float Val)
{
	if (Val == 0) return;

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

	GEngine->AddOnScreenDebugMessage(TextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(TextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);

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

			// Setup parameters for the object to place
			if (Mesh != nullptr)
			{
				Mesh->SetMobility(EComponentMobility::Movable);
				Mesh->SetCollisionProfileName("OverlapOnlyPawn");
				Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				Mesh->SetSimulatePhysics(true);
				Mesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
				Mesh->bGenerateOverlapEvents = true;
			}
			// ****************************************

			ListOfRefillTitemsPlacedInWorld.Add(elem);

			if (FTagStatics::HasKeyValuePair(elem, "Refill", "Hookable", "True")) {
				CreateConstraintsForHookableItems(elem);
			}

			PhantomItems.RemoveSwap(elem);
			ObjectPool.RemoveSwap(elem); // Also remove item from the object pool since it isn't available anymore for phantom items
		}
	}
}

void UCPlaceItem::RemoveItems()
{
	for (auto & elem : SelectedItems)
	{
		// Check for a hooked item and remove it
		UActorComponent* HookComponent = FindHookOfItem(elem);
		if (HookComponent != nullptr && ItemsOnHook.Contains(HookComponent)) {
			ItemsOnHook[HookComponent].Remove(elem);
		}
		// *** *** *** 

		ListOfRefillTitemsPlacedInWorld.Remove(elem);
		elem->Destroy();
	}

	DeselectItems();
}

void UCPlaceItem::CheckPlayerInput()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	RaytraceResults = StartRaytrace();

	if (RaytraceResults.GetActor() != nullptr)
	{
		if (CurrentInteractionState == InteractionState::PLACING) {

			DeselectItems();
			if (FocusedItem != nullptr)	GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false);

			if (SetOfShelves.Contains(RaytraceResults.GetActor()))
			{
				// We hit a shelf and we are in placing mode
				LastFocusedHook = nullptr; // Reset hook
				DisplayPhantomItem(RaytraceResults);
			}
			else {
				// Check if it is a hook
				UActorComponent* Component = Cast<UActorComponent>(RaytraceResults.GetComponent());
				if (Component != nullptr && SetOfHooks.Contains(Component)) {
					Stacking = 1; // Reset stacking
					DisplayPhantomHookItem(RaytraceResults);
				}
				else {
					// We did not hit a shelf or a hook
					DeactivatePhantomItems();
				}
			}
		}
		else if (CurrentInteractionState == InteractionState::SELECTING)
		{
			if (ListOfRefillTitemsPlacedInWorld.Contains(RaytraceResults.GetActor())) {
				// We hit a refill object while in selection mode
				DeactivatePhantomItems();

				// Check if it we focused on a new item or activated row resp wholeShelf mode
				bool bIsNewSelection = FocusedItem != RaytraceResults.GetActor() || bLeftShiftWasPressed || bLeftControlWasPressed || bLeftShiftWasReleased || bLeftControlWasReleased;

				if (bIsNewSelection)
				{
					if (FocusedItem != nullptr)
					{
						GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false); // Deactivate outline of the previous item
						SelectedItems.Remove(FocusedItem); // Remove the old item from the list of selected items
					}

					FocusedItem = RaytraceResults.GetActor(); // RaytraceResults.first.GetActor(); // Assign new item as focused item
					// GetStaticMesh(FocusedItem)->SetRenderCustomDepth(true); // Reactivate outline effect for new item


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
						SelectItem(FocusedItem);
						//SelectedItems.Add(FocusedItem); 
					}
				}

			}
			else {
				// We hit something that isnt a refills object
				DeselectItems();

				if (FocusedItem != nullptr)
				{
					GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false);
					FocusedItem = nullptr;
				}
			}
		}
		else
		{
			DeselectItems();

			// We are not in selecting mode anymore
			if (FocusedItem != nullptr)
			{
				// Disable last focused item
				GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false);
				//SelectedItems.Remove(FocusedItem);
				FocusedItem = nullptr;
			}
		}
	}
	else {
		// We didn't hit anything
		DeactivatePhantomItems();

		// We are not in selecting mode anymore
		if (FocusedItem != nullptr)
		{
			// Disable last focused item
			DeselectItems();
			GetStaticMesh(FocusedItem)->SetRenderCustomDepth(false);
			//SelectedItems.Remove(FocusedItem);
			FocusedItem = nullptr;
		}
	}
}

void UCPlaceItem::DisplayPhantomItem(FHitResult Hit)
{
	if (Character == nullptr) return;
	if (ItemTemplate == nullptr) return;

	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
	ItemTemplate->K2_SetActorRotation(Rotation, true);// Set rotation before checking for extends

	// Get position and bounds of the item
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	ItemTemplate->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	FVector DeltaOfPivotToCenter = ItemOrigin - ItemTemplate->GetActorLocation(); // Get the difference between pivot and center of blocking volume

	ItemTemplate->SetActorRotation(BlockingVol->GetActorRotation() + FRotator::MakeFromEuler(FVector(0, 0, 180)) + Rotation); // Set item's rotation to blocking volume rotation + the item rotation

	FVector BlockingVolumeOrigin;
	FVector NotUsed; // We don't need the extends of the blocking volume's AABB

	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, NotUsed);

	// Use the extends of the brush. This is the extend of Object Oriented Bounding Box
	FVector BlockingVolumeExtend = BlockingVol->Brush->Bounds.BoxExtent;

	// Get direction of x, y and z axis
	FVector XAxis = BlockingVol->GetActorForwardVector();
	FVector YAxis = BlockingVol->GetActorRightVector();
	FVector ZAxis = BlockingVol->GetActorUpVector();

	// Calculating new position
	FVector Position = Hit.ImpactPoint;

	// BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z -> The lower edge of the item is at the height of the pivot point of the blocking volume
	FVector NewPosition = FVector(Position.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z) - DeltaOfPivotToCenter;
	//	bool bNewInputMode = bLeftShiftWasPressed || bLeftControlWasPressed || bLeftShiftWasReleased || bLeftControlWasReleased;

	if (LocationToPlaceItem != NewPosition || bInputHasBeenChanged) { // Check if we need to update the phantom items
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

		FVector StartVector;

		if (bLeftShiftIsHeldDown)
		{
			// Placing a row

			// Calculate how many items we can place in a row, based on the blocking volume and item extends
			AmountOfItemsPossibleX = FMath::FloorToInt((2 * BlockingVolumeExtend.X) / (2 * ItemBoundExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));

			StartVector = GetRowStartPoint(BlockingVol, ItemTemplate, NewPosition); // This doesn't need DeltaOfPivotToCenter

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
			AmountOfItemsPossibleX = FMath::FloorToInt(BlockingVolumeExtend.X / (ItemBoundExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));
			AmountOfItemsPossibleY = FMath::FloorToInt(BlockingVolumeExtend.Y / (ItemBoundExtend.Y * (SpacingY + MINIMUM_GAP_BETWEEN_ITEMS)));

			// The lower left corner of the blocking volume
			FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis - BlockingVolumeExtend.Z * ZAxis;

			StartVector = BasePointPlane + ItemBoundExtend.X * XAxis + ItemBoundExtend.Y * YAxis + (ItemBoundExtend.Z + BlockingVolumeExtend.Z) * ZAxis - DeltaOfPivotToCenter;

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

		// Now show the items
		for (size_t s = 0; s < Stacking; s++)
		{
			for (size_t i = 0; i < AmountOfItemsPossibleX; i++)
			{
				for (size_t j = 0; j < AmountOfItemsPossibleY; j++)
				{
					// Get clone of the item to display
					AActor* Clone = GetCloneActor(ItemTemplate);

					if (Clone == nullptr) return;

					PhantomItems.Add(Clone);

					// The points within the blocking volume
					FVector RelativePointOnXAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingX) * XAxis * 2 * ItemBoundExtend.X * i;
					FVector RelativePointOnYAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingY) * YAxis * 2 * ItemBoundExtend.Y * j;
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
}

void UCPlaceItem::DisplayPhantomHookItem(FHitResult Hit)
{
	if (ItemTemplate == nullptr) return;
	if (FTagStatics::HasKeyValuePair(ItemTemplate, "Refill", "Hookable", "True") == false) return; // The item we try to place on the hook is not a hookable item

	// Deactivate all former phantom items
	for (auto &elem : PhantomItems)
	{
		elem->SetActorHiddenInGame(true);
		elem->SetActorEnableCollision(false);
	}
	PhantomItems.Empty();

	UActorComponent* HoleComponentAsActorComp = ItemTemplate->GetComponentByClass(UHoleTabComponent::StaticClass());
	if (HoleComponentAsActorComp == nullptr) return;
	UHoleTabComponent* HoleComponent = Cast<UHoleTabComponent>(HoleComponentAsActorComp);

	// Calculate the position of the item on the hook, depending on the HoleTab's relative position

	FVector CenterOfBox = Hit.GetComponent()->Bounds.GetBox().GetCenter();
	float HookExtendOnX = Hit.GetComponent()->Bounds.GetBox().GetExtent().X;

	ItemTemplate->K2_SetActorRotation(Rotation, true);// Set rotation before checking for extends

	FVector ItemOrigin;
	FVector ItemExtend;
	ItemTemplate->GetActorBounds(false, ItemOrigin, ItemExtend);

	int AmountOfItemsToPlace = 1;

	ItemTemplate->SetActorRotation(Hit.GetComponent()->GetComponentRotation() + Rotation); // Set item's rotation to blocking volume rotation + the item rotation
	// Calculate amount of items 

	if (bLeftShiftIsHeldDown) {
		AmountOfItemsToPlace = FMath::FloorToInt((2 * HookExtendOnX) / (2 * ItemExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));
	}

	FVector XAxis = Hit.GetComponent()->GetRightVector();
	FVector FrontItemPosition = CenterOfBox + HookExtendOnX * XAxis - HoleComponent->RelativeLocation;

	for (size_t i = 0; i < AmountOfItemsToPlace; i++)
	{
		FVector RelativePointOnXAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingX) * (-XAxis) * 2 * ItemExtend.X * i;

		AActor* PhantomItem = GetCloneActor(ItemTemplate);
		PhantomItems.Add(PhantomItem);
		PhantomItem->SetActorLocation(FrontItemPosition + RelativePointOnXAxis);

		bItemCanBePlaced = CheckCollisions(PhantomItem);
	}

	LastFocusedHook = Hit.GetComponent();
}

void UCPlaceItem::SelectItem(AActor * Item)
{
	SelectedItems.Add(Item);

	AStaticMeshActor* MeshActorOfItem = Cast<AStaticMeshActor>(Item);
	if (MeshActorOfItem != nullptr) {
		MeshActorOfItem->GetStaticMeshComponent()->SetRenderCustomDepth(true);
	}
}

void UCPlaceItem::SelectRow()
{
	UActorComponent* HookComponent = FindHookOfItem(FocusedItem);

	if (HookComponent != nullptr) {
		// It is an item on a hook
		if (ItemsOnHook.Contains(HookComponent)) {
			DeselectItems();

			TArray<AActor*> AllItemsOnTheSameHook = ItemsOnHook[HookComponent];

			for (auto& ItemOnHook : AllItemsOnTheSameHook) {
				SelectItem(ItemOnHook);
			}

			UE_LOG(LogTemp, Log, TEXT("%s: Found %i hooked items"), *FString(__FUNCTION__), SelectedItems.Num());
		}
	}
	else {
		// It is a shelf item
		ABlockingVolume* BlockingVol = FindShelf(FocusedItem); // Get the shelf the focused item is standing on

		if (BlockingVol == nullptr)
		{
			if (SelectedItems.Contains(FocusedItem) == false)
			{
				SelectedItems.Add(FocusedItem); // There is no blocking volume. Select only focused item
			}

			return; // That wasn't a blocking volume we hit 
		}

		TArray<FHitResult> Hits;

		// Start point is the point of the focused item
		FVector StartVectorTemp = GetRowStartPoint(BlockingVol, FocusedItem, FocusedItem->GetActorLocation());
		FVector StartVector = FVector(
			StartVectorTemp.X,
			StartVectorTemp.Y,
			BlockingVol->GetActorLocation().Z + 2 * BlockingVol->Brush->Bounds.BoxExtent.Z
		);

		// End vector is a point at the end of the row. Starting from first point 'StartVecttor' and going from there along the x axis
		FVector EndVector = StartVector + 2 * BlockingVol->Brush->Bounds.BoxExtent.X * BlockingVol->GetActorForwardVector();

		FCollisionQueryParams TraceParams;
		TraceParams.TraceTag = TAG_ITEM; //  Only hit Refill Items

		GetWorld()->LineTraceMultiByChannel(Hits, StartVector, EndVector, ECollisionChannel::ECC_Pawn, TraceParams);

		for (auto& elem : Hits)
		{
			AActor* HitActor = elem.GetActor();

			if (HitActor != nullptr && ListOfRefillTitemsPlacedInWorld.Contains(HitActor))
			{
				//UStaticMeshComponent* MeshComp = GetStaticMesh(HitActor);
				//if (MeshComp != nullptr)
				//{
				//	MeshComp->SetRenderCustomDepth(true);
				//}

				CheckStackedNeighbour(HitActor); // Check if this item has any other item stacked onto it
				//SelectedItems.Add(HitActor); // Select the item
				SelectItem(HitActor);
			}
		}

		if (bIsDebugMode)
		{
			DrawDebugLine(GetWorld(), StartVector, EndVector, FColor::Blue, false, 3, 5);
		}
	}



}

void UCPlaceItem::SelectAllItems()
{
	ABlockingVolume* BlockingVol = FindShelf(FocusedItem);

	if (BlockingVol == nullptr)
	{
		if (SelectedItems.Contains(FocusedItem) == false)
		{
			SelectedItems.Add(FocusedItem); // Only select focused item
		}

		return; // That wasn't a blocking volume we hit
	}

	TArray<AActor*> ItemsOnShelf;
	BlockingVol->GetOverlappingActors(ItemsOnShelf);

	for (auto & HitActor : ItemsOnShelf)
	{
		if (ListOfRefillTitemsPlacedInWorld.Contains(HitActor))
		{
			//UStaticMeshComponent* MeshComp = GetStaticMesh(HitActor);
			//if (MeshComp != nullptr)
			//{
			//	MeshComp->SetRenderCustomDepth(true);
			//}

			CheckStackedNeighbour(HitActor); // Check if this item has any other item stacked onto it
			// SelectedItems.Add(HitActor); // Select item
			SelectItem(HitActor);
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

	if (FocusedItem != nullptr && bLeftShiftIsHeldDown == false && bLeftControlIsHeldDown == false)
	{
		GetStaticMesh(FocusedItem)->SetRenderCustomDepth(true);
		SelectedItems.Add(FocusedItem);
	}
}

void UCPlaceItem::CreateConstraintsForHookableItems(AActor* HookableItem)
{
	if (LastFocusedHook == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("%s: Trying to place an item on a hook but hook was not found"), *FString(__FUNCTION__));
	}
	else {
		UPhysicsConstraintComponent* ConstraintComponent = NewObject<UPhysicsConstraintComponent>(HookableItem);

		if (ConstraintComponent == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("%s: Could not create constraint component on %s"), *FString(__FUNCTION__), *HookableItem->GetName());
			return;
		}

		UPrimitiveComponent* HoleTabComponent = Cast<UPrimitiveComponent>(HookableItem->GetComponentByClass(UHoleTabComponent::StaticClass()));

		if (HoleTabComponent == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("%s: HoleTab Component not found on actor %s"), *FString(__FUNCTION__),*HookableItem->GetName());
			return;
		}

		UPrimitiveComponent* ItemStaticMeshRootComponent = Cast<UPrimitiveComponent>(HookableItem->GetRootComponent());

		ConstraintComponent->SetWorldLocation(HoleTabComponent->GetComponentLocation());

		ConstraintComponent->AttachTo(HookableItem->GetRootComponent(), NAME_None, EAttachLocation::KeepWorldPosition);
		ConstraintComponent->SetConstrainedComponents(ItemStaticMeshRootComponent, NAME_None, LastFocusedHook, NAME_None);

		ConstraintComponent->SetDisableCollision(true);
		ConstraintComponent->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
		ConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 5.0f); // TODO Hardcoded limits
		ConstraintComponent->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 5.0f);

		UE_LOG(LogTemp, Log, TEXT("%s: Added constraint"), *FString(__FUNCTION__));

		if (ItemsOnHook.Contains(LastFocusedHook)) {
			ItemsOnHook[LastFocusedHook].Add(HookableItem);
		}
		else {
			TArray<AActor*> NewActorArray;
			NewActorArray.Add(HookableItem);
			ItemsOnHook.Add(LastFocusedHook, NewActorArray);
		}

	}
}

UStaticMeshComponent* UCPlaceItem::GetStaticMesh(AActor* Actor)
{
	AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);

	if (MeshActor != nullptr) {
		return MeshActor->GetStaticMeshComponent();
	}
	else {
		return nullptr;
	}
}

void UCPlaceItem::IncreaseSpacingStep()
{
	SpacingStep *= 2;
	if (SpacingStep <= 0) SpacingStep = 0.1f;
	GEngine->AddOnScreenDebugMessage(TextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / SpacingStep), false);
}

void UCPlaceItem::DecreaseSpacingStep()
{
	SpacingStep /= 2;

	GEngine->AddOnScreenDebugMessage(TextID_SpacingStep, 100000.0f, FColor::Yellow, "Spacing: x" + FString::SanitizeFloat(1 / SpacingStep), false);
	bInputHasBeenChanged = true;
}

void UCPlaceItem::IncreaseStacking()
{
	Stacking++;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::DecreaseStacking()
{
	Stacking--;
	if (Stacking < 1) Stacking = 1;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::StepRotation()
{
	Rotation += FRotator(0, 90, 0);
	bInputHasBeenChanged = true;
}

void UCPlaceItem::TogglePlacingRemovingState()
{
	if (CurrentInteractionState == InteractionState::PLACING)
	{
		CurrentInteractionState = InteractionState::SELECTING;
		DeactivatePhantomItems();

		GEngine->AddOnScreenDebugMessage(TextID_PlacingSelectingMode, 1000000.0f, FColor::Blue, "Current Mode: SELECTING", false);
	}
	else
	{
		CurrentInteractionState = InteractionState::PLACING;
		FocusedItem = nullptr;
		GEngine->AddOnScreenDebugMessage(TextID_PlacingSelectingMode, 1000000.0f, FColor::Blue, "Current Mode: PLACING", false);
	}
}

AActor * UCPlaceItem::GetCloneActor(AActor * ActorToClone)
{
	for (auto& object : ObjectPool) // First check the object pool if there is any unused object
	{
		if (object->bHidden) // Take the first object that is hidden
		{
			object->SetActorHiddenInGame(false);
			object->SetActorEnableCollision(true);

			UStaticMeshComponent* MeshComponent = GetStaticMesh(object);
			UStaticMesh* MeshOfActorToClone = GetStaticMesh(ActorToClone)->GetStaticMesh();

			if (MeshComponent->GetStaticMesh() != MeshOfActorToClone)
			{
				MeshComponent->SetStaticMesh(MeshOfActorToClone);
			}

			MeshComponent->SetMaterial(0, ItemMaterial);

			MeshComponent->SetCollisionProfileName("OverlapAll");
			MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			MeshComponent->SetSimulatePhysics(false);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			MeshComponent->bGenerateOverlapEvents = true;

			object->K2_SetActorRotation(ItemTemplate->GetActorRotation(), false);

			return object;
		}
	}

	// If we came here, we don't have an unused objects in the object pool. We create a new one
	FActorSpawnParameters Parameters;
	Parameters.Template = ActorToClone;
	AActor* Clone = GetWorld()->SpawnActor<ARRefillObject>(ARRefillObject::StaticClass(), Parameters);

	UStaticMeshComponent* MeshComponent = GetStaticMesh(Clone);

	ItemMaterial = MeshComponent->GetMaterial(0);
	MeshComponent->SetCollisionProfileName("OverlapAll"); // A phantom item shouldn't collide with anything but overlap
	MeshComponent->bGenerateOverlapEvents = true;

	ObjectPool.Add(Clone);

	if (Clone->Tags.Contains(TAG_ITEM) == false)
	{
		Clone->Tags.Add(TAG_ITEM); // Set the item tag 
	}

	return Clone;

}

bool UCPlaceItem::CheckCollisions(AActor * Actor, UActorComponent* IgnoredComponent)
{
	UStaticMeshComponent* ActorMesh = GetStaticMesh(Actor);

	bool bActorPlacable = true;
	GetStaticMesh(Actor)->bGenerateOverlapEvents = true;

	TArray<UPrimitiveComponent*> HitComponents;
	Actor->GetOverlappingComponents(HitComponents);

	for (auto& elem : HitComponents) {
		if (SetOfHooks.Contains(elem) == false && SetOfShelves.Contains(elem->GetOwner()) == false && PhantomItems.Contains(elem->GetOwner()) == false) {
			// It's not a shelf or hook and not another phantom item
			UE_LOG(LogTemp, Log, TEXT("%s: Overlapping with %s of actor %s"), *FString(__FUNCTION__), *elem->GetName(), *elem->GetOwner()->GetName());
			bActorPlacable = false;
			break;
		}
	}

	if (bActorPlacable == false)
	{
		// Apply the red material to every phantom object
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
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: No colision material to apply"), *FString(__FUNCTION__));
		}
		// *********************************************
	}
	else
	{
		// There was no collision for this item. 
		if (ItemMaterial != nullptr)
		{
			for (auto &elem : PhantomItems)
			{
				// Color it normal
				UStaticMeshComponent* Mesh = GetStaticMesh(elem);

				if (Mesh != nullptr)
				{
					Mesh->SetMaterial(0, ItemMaterial); // Set the default material for this item
				}
			}
		}
	}

	return bActorPlacable;
}

void UCPlaceItem::OnNewItemSpawned(AActor* SpawnedActor)
{
	if (SpawnedActor != nullptr)
	{
		ItemMaterial = GetStaticMesh(SpawnedActor)->GetMaterial(0);
	} 

	ItemTemplate = SpawnedActor; // Get the selected item from the asset loader

	if (ItemTemplate != nullptr)
	{
		GetStaticMesh(ItemTemplate)->SetMaterial(0, ItemMaterial);
		
		if (CurrentInteractionState == InteractionState::SELECTING) TogglePlacingRemovingState();
	}
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
		if (StackedActor != nullptr && ListOfRefillTitemsPlacedInWorld.Contains(StackedActor)/*StackedActor->Tags.Contains(ItemTag)*/)
		{
			//UStaticMeshComponent* MeshCompStackItem = GetStaticMesh(StackedActor);
			//if (MeshCompStackItem != nullptr)
			//{
			//	MeshCompStackItem->SetRenderCustomDepth(true);
			//}

			if (SelectedItems.Contains(StackedActor) == false)
			{
				//SelectedItems.Add(StackedActor);
				SelectItem(StackedActor);
				CheckStackedNeighbour(StackedActor); // Call recursivly
			}
		}
	}
}

ABlockingVolume * UCPlaceItem::FindShelf(AActor * FromActor)
{
	ABlockingVolume* BlockingVol = nullptr;

	TArray<FHitResult> BlockingVolumeHits;
	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = FName(TAG_SHELF);
	TraceParams.AddIgnoredActor(FocusedItem);

	FVector StartVector = FocusedItem->GetActorLocation();
	FVector EndVector = StartVector - FocusedItem->GetActorUpVector() * 100.0f; // Quite long vector

	GetWorld()->LineTraceMultiByChannel(BlockingVolumeHits, StartVector, EndVector, ECollisionChannel::ECC_Camera, TraceParams);

	for (auto & elem : BlockingVolumeHits)
	{
		AActor* HitActor = elem.GetActor();

		if (HitActor != nullptr && SetOfShelves.Contains(HitActor))
		{
			BlockingVol = Cast<ABlockingVolume>(HitActor);
			break;
		}
	}

	return BlockingVol;
}

UActorComponent * UCPlaceItem::FindHookOfItem(AActor * Item)
{
	UActorComponent* HoleTabComponent = Item->GetComponentByClass(UHoleTabComponent::StaticClass());

	if (HoleTabComponent != nullptr) {

		UActorComponent* HookConstraintAsActorComp = FocusedItem->GetComponentByClass(UPhysicsConstraintComponent::StaticClass());
		if (HookConstraintAsActorComp != nullptr) {
			UPhysicsConstraintComponent* HookConstraint = Cast<UPhysicsConstraintComponent>(HookConstraintAsActorComp);

			UActorComponent* HookComponent = nullptr;

			// Get the UActorComponent of the hook
			if (HookConstraint != nullptr) {
				if (HooknamesToHookComponent.Contains(HookConstraint->ComponentName1.ComponentName)) {
					HookComponent = HooknamesToHookComponent[HookConstraint->ComponentName1.ComponentName];
				}
				else if (HooknamesToHookComponent.Contains(HookConstraint->ComponentName2.ComponentName)) {
					HookComponent = HooknamesToHookComponent[HookConstraint->ComponentName2.ComponentName];
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("%s: No hook was found"), *FString(__FUNCTION__));
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("%s: No hook constraint was found"), *FString(__FUNCTION__));
			}

			return HookComponent;
		}
	}

	return nullptr;
}

void UCPlaceItem::DeactivatePhantomItems()
{
	for (auto &elem : PhantomItems)
	{
		elem->SetActorHiddenInGame(true);
		elem->SetActorEnableCollision(false);
	}

	PhantomItems.Empty();
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

void UCPlaceItem::OnKeyRowModePressed()
{
	bLeftShiftWasPressed = true;
	bLeftShiftIsHeldDown = true;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::OnKeyRowModeReleased()
{
	bLeftShiftWasReleased = true;
	bLeftShiftIsHeldDown = false;
	bInputHasBeenChanged = true;

	DeselectItems();
}

void UCPlaceItem::OnKeyWholeShelfModePressed()
{
	bLeftControlWasPressed = true;
	bLeftControlIsHeldDown = true;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::OnKeyWholeShelfModeReleased()
{
	bLeftControlWasReleased = true;
	bLeftControlIsHeldDown = false;
	bInputHasBeenChanged = true;

	DeselectItems();
}

void UCPlaceItem::ResetInputKeys() {
	bLeftShiftWasPressed = false;
	bLeftShiftWasReleased = false;

	bLeftControlWasPressed = false;
	bLeftControlWasReleased = false;

	bInputHasBeenChanged = false;
}


