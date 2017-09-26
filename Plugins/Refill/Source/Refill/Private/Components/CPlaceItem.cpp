// Fill out your copyright notice in the Description page of Project Settings.
#define TAG_KEY_SHELF "Shelf"
#define TAG_SHELF "Refill;Shelf,True;"

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

	// Find ItemManager
	for (TActorIterator<AItemManager>Itr(GetWorld()); Itr; ++Itr)
	{
		ItemManager = *Itr;
		break;
	}

	if (ItemManager == nullptr) {
		// Let the game crash if no ItemManager was found
		UE_LOG(LogTemp, Fatal, TEXT("%s %s No ItemManager instance was found."), *FString(__FUNCTION__), *FString::FromInt(__LINE__));
		return;
	}

	// Get all actors with specific tag
	ItemManager->ListOfRefillTitemsPlacedInWorld = FTagStatics::GetActorsWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_ITEM, "True");
	SetOfShelves = FTagStatics::GetActorSetWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_SHELF, "True");

	UE_LOG(LogTemp, Log, TEXT("%s: Items found %i"), *FString(__FUNCTION__), ItemManager->ListOfRefillTitemsPlacedInWorld.Num());
	UE_LOG(LogTemp, Log, TEXT("%s: Shelves found %i"), *FString(__FUNCTION__), SetOfShelves.Num());
	// *********************************************** 

	if (AssetLoader == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(TextID_Error, 100000.0f, FColor::Red, "UCPlaceItem::BeginPlay: Please assign an AssetLoader actor", false);
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		return;
	}
	else {
		// Bind OnNewItemSpawned function to delegate
		UE_LOG(LogTemp, Log, TEXT("%s: Bind OnNewItemSpawned"), *FString(__FUNCTION__));
		AssetLoader->OnItemSpawend.AddDynamic(this, &UCPlaceItem::OnNewItemSpawned);
	}

	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()); // Assing the hud
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

	// Add visual feedback in which state the player is
	if (bRowModeKeyIsHeldDown) {
		GEngine->AddOnScreenDebugMessage(TextID_RowMode, 0.1f, FColor::Green, "Row-Mode", false);
	}
	else if (bWholeShelfModeKeyIsHeldDown) {

		if (bWholeShelfModeKeyIsHeldDown) GEngine->AddOnScreenDebugMessage(TextID_WholeShelfMode, 0.1f, FColor::Green, "WholeShelf-Mode", false);
	}

	// Reset the input key variables each tick
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
		// Ignore all placed RefillITems in the world so we can place new items behind them
		TraceParams.AddIgnoredActors(ItemManager->ListOfRefillTitemsPlacedInWorld);
	}

	TraceParams.AddIgnoredActor(GetOwner()); // Ignore the player
	TraceParams.AddIgnoredActor(ItemTemplate); // Ignore the item in our hand

	// Add phantom items to be ignored by the raytrace
	TArray<AActor*> IgnoredPhantomItems;
	for (auto& PhantomElem : PhantomItems) {
		IgnoredPhantomItems.Add(PhantomElem);
	}
	TraceParams.AddIgnoredActors(IgnoredPhantomItems);
	// *** *** *** *** *** *** *** *** *** *** *** ***

	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldStatic, TraceParams);

	return Hit;
}

void UCPlaceItem::ChangeSpacing(const float Val)
{
	if (Val == 0) return;

	if (bRowModeKeyIsHeldDown)
	{
		SpacingX += Val / SpacingStep; // Add the value of the mouse wheel 

		if (SpacingX < 1) SpacingX = 1; // Never get lower than 1

		PlaceItemLocation = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	if (bWholeShelfModeKeyIsHeldDown)
	{
		SpacingY += Val / SpacingStep;

		if (SpacingY < 1) SpacingY = 1;

		PlaceItemLocation = FVector::ZeroVector; // Reset this vector to force a new raytrace
	}

	// Visual feedback
	GEngine->AddOnScreenDebugMessage(TextID_XSpacing, 100000.0f, FColor::Yellow, "Row Spacing: " + FString::SanitizeFloat(SpacingX), false);
	GEngine->AddOnScreenDebugMessage(TextID_YSpacing, 100000.0f, FColor::Yellow, "Column Spacing: " + FString::SanitizeFloat(SpacingY), false);

}

void UCPlaceItem::PlaceItems()
{
	if (bItemCanBePlaced)
	{
		TArray<ARRefillObject*> ItemsToPlace = PhantomItems;

		// For each Phantom item 
		for (auto &elem : ItemsToPlace)
		{
			UStaticMeshComponent* Mesh = elem->GetStaticMeshComponent();

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

			// Add to the list of all items in the world
			ItemManager->ListOfRefillTitemsPlacedInWorld.Add(elem);

			// Check if it can be placed on a hook
			if (elem->ObjectInfo.bCanBeHookedUp) {
				if (ItemManager != nullptr) {
					ItemManager->AddItemToHook(LastFocussedHook, elem); // Add the item on the hook
				}
			}

			// We  need to remove the items from the phantom item array so they won't get destroyed the next time we move the mouse
			PhantomItems.RemoveSwap(elem);

			// Also remove item from the object pool since it isn't available anymore for phantom items
			ObjectPool.RemoveSwap(elem);
		}
	}
}

void UCPlaceItem::RemoveItems()
{
	for (auto & elem : SelectedItems)
	{
		// Check for a hooked item and remove it
		if (ItemManager != nullptr) {
			UActorComponent* HookComponent = ItemManager->FindHookOfItem(elem);
			ItemManager->RemoveItemFromHook(HookComponent, elem); // Remove the item pointer from the hook 

		}

		// Remove the item from the list of all items in the world
		ItemManager->ListOfRefillTitemsPlacedInWorld.Remove(elem);

		// Finally destroy the actor of this item
		elem->Destroy();
	}

	// Deselect items
	DeselectItems();
}

void UCPlaceItem::CheckPlayerInput()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	RaytraceResults = StartRaytrace();

	if (RaytraceResults.GetActor() != nullptr)
	{
		if (CurrentInteractionState == InteractionState::PLACING) {
			// We are in placing mode

			DeselectItems(); // First deselect all previous items

			if (FocussedItem != nullptr) GetStaticMesh(FocussedItem)->SetRenderCustomDepth(false); // Un-higlight the item

			if (SetOfShelves.Contains(RaytraceResults.GetActor()))
			{
				// We hit a shelf and we are in placing mode
				LastFocussedHook = nullptr; // Reset hook
				DisplayPhantomItem(RaytraceResults); // Show PhantomItem on the shelf
			}
			else {
				// Check if it is a hook

				UActorComponent* Component = Cast<UActorComponent>(RaytraceResults.GetComponent());
				if (Component != nullptr && ItemManager != nullptr && ItemManager->SetOfHooks.Contains(Component)) {
					Stacking = 1; // Reset stacking
					DisplayPhantomHookItem(RaytraceResults); // Show PhantomItem on the hook
				}
				else {
					// We did not hit a shelf or a hook. Deactivate PhantomItems
					DeactivatePhantomItems();
				}
			}
		}
		else if (CurrentInteractionState == InteractionState::SELECTING)
		{
			// We're in Selection mode
			if (ItemManager->ListOfRefillTitemsPlacedInWorld.Contains(RaytraceResults.GetActor())) {
				// We hit a refill object while in selection mode. Deactivate all PhantomItems
				DeactivatePhantomItems();

				// Check if it we focused on a new item or activated row- or wholeShelf mode
				bool bIsNewSelection = FocussedItem != RaytraceResults.GetActor() || bRowModeKeyWasPressed || bWholeShelfModeKeyWasPressed || bRowModeKeyWasReleased || bWholeShelfModeKeyWasReleased;

				if (bIsNewSelection)
				{
					if (FocussedItem != nullptr)
					{
						GetStaticMesh(FocussedItem)->SetRenderCustomDepth(false); // Deactivate outline of the previous item
						SelectedItems.Remove(FocussedItem); // Remove the old item from the list of selected items
					}

					FocussedItem = Cast<ARRefillObject>(RaytraceResults.GetActor()); // Assign new item as focused item

					if (bRowModeKeyIsHeldDown)
					{
						DeselectItems(); // Deselect all old items
						SelectRow(); // Select the new row
					}
					else if (bWholeShelfModeKeyIsHeldDown)
					{
						DeselectItems(); // Deselect all old items
						SelectAllItems(); // Select all items on the shelf
					}
					else
					{
						SelectItem(FocussedItem); // We only select the one item the player is focussing
					}
				}

			}
			else {
				// We hit something that isn't a refills object
				DeselectItems();

				if (FocussedItem != nullptr)
				{
					GetStaticMesh(FocussedItem)->SetRenderCustomDepth(false); // Deactivate hinglighting
					FocussedItem = nullptr;
				}
			}
		}
		else
		{
			DeselectItems();

			// We are not in selecting mode anymore
			if (FocussedItem != nullptr)
			{
				// Disable last focused item
				GetStaticMesh(FocussedItem)->SetRenderCustomDepth(false);
				FocussedItem = nullptr;
			}
		}
	}
	else {
		// We didn't hit anything
		DeactivatePhantomItems();

		// We are not in selecting mode anymore
		if (FocussedItem != nullptr)
		{
			// Disable last focused item
			DeselectItems();
			GetStaticMesh(FocussedItem)->SetRenderCustomDepth(false);
			FocussedItem = nullptr;
		}
	}
}

void UCPlaceItem::DisplayPhantomItem(FHitResult Hit)
{
	if (ItemTemplate == nullptr) return;

	ABlockingVolume* BlockingVol = Cast<ABlockingVolume>(Hit.GetActor());
	ItemTemplate->K2_SetActorRotation(PlaceItemRotation, true);// Set rotation before checking for extends

	// Get position and bounds of the item
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	ItemTemplate->GetActorBounds(false, ItemOrigin, ItemBoundExtend);

	// Get the difference between pivot and center of blocking volume
	FVector DeltaOfPivotToCenter = ItemOrigin - ItemTemplate->GetActorLocation();

	// Set item's rotation to blocking volume rotation + the item rotation
	ItemTemplate->SetActorRotation(BlockingVol->GetActorRotation() + FRotator::MakeFromEuler(FVector(0, 0, 180)) + PlaceItemRotation); // TODO Hardcoded 180

	// *** Get blocking volume bounds ***
	FVector BlockingVolumeOrigin;
	FVector NotUsed; // We don't need the extends of the blocking volume's AABB
	BlockingVol->GetActorBounds(false, BlockingVolumeOrigin, NotUsed);
	// *** *** *** *** *** *** *** *** **

	// Use the extends of the brush. This is the extend of Object Oriented Bounding Box
	FVector BlockingVolumeExtend = BlockingVol->Brush->Bounds.BoxExtent;

	// Get direction of x, y and z axis
	FVector XAxis = BlockingVol->GetActorForwardVector();
	FVector YAxis = BlockingVol->GetActorRightVector();
	FVector ZAxis = BlockingVol->GetActorUpVector();

	// Calculating new position
	FVector Position = Hit.ImpactPoint;

	// Calculate new position
	FVector NewPosition = FVector(Position.X, Position.Y, BlockingVolumeOrigin.Z + ItemBoundExtend.Z - BlockingVolumeExtend.Z) - DeltaOfPivotToCenter;

	// Check if we need to update the phantom items
	if (PlaceItemLocation != NewPosition || bInputHasBeenChanged) {
		PlaceItemLocation = NewPosition;

		// Deactivate all former phantom items
		for (auto &elem : PhantomItems)
		{
			elem->SetActorHiddenInGame(true);
			elem->SetActorEnableCollision(false);
		}

		// Empty the list of all PhantomItems
		PhantomItems.Empty();

		// Initialize the amounts of items to 1
		int AmountOfItemsPossibleX = 1;
		int AmountOfItemsPossibleY = 1;

		FVector StartVector;

		if (bRowModeKeyIsHeldDown)
		{
			// Placing a row

			// Calculate how many items we can place in a row, based on the blocking volume and item extends
			AmountOfItemsPossibleX = FMath::FloorToInt((2 * BlockingVolumeExtend.X) / (2 * ItemBoundExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));

			// The new start position from which the row is build up. This doesn't need DeltaOfPivotToCenter
			StartVector = GetRowStartPoint(BlockingVol, ItemTemplate, NewPosition);

			if (bIsDebugMode)
			{
				FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis;
				DrawDebugPoint(GetWorld(), StartVector, 5, FColor::Blue, false, 3);
				DrawDebugLine(GetWorld(), BasePointPlane, BasePointPlane + BlockingVolumeExtend.Y * YAxis * 2, FColor::Blue, false, 3, 5);
			}
		}
		else if (bWholeShelfModeKeyIsHeldDown)
		{
			// Fill with items

			// Calculate the amount of items we can place on the shelf in x and y axis (column and row)
			AmountOfItemsPossibleX = FMath::FloorToInt(BlockingVolumeExtend.X / (ItemBoundExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));
			AmountOfItemsPossibleY = FMath::FloorToInt(BlockingVolumeExtend.Y / (ItemBoundExtend.Y * (SpacingY + MINIMUM_GAP_BETWEEN_ITEMS)));

			// The lower left corner of the blocking volume
			FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis - BlockingVolumeExtend.Z * ZAxis;

			// The new start position from which the items fill up the shelf
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
			// We're placing a single item
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
					ARRefillObject* Clone = GetCloneActor(ItemTemplate);

					if (Clone == nullptr) return;

					// Add this clone to the list of PhantomItems
					PhantomItems.Add(Clone);

					// The points within the blocking volume
					FVector RelativePointOnXAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingX) * XAxis * 2 * ItemBoundExtend.X * i;
					FVector RelativePointOnYAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingY) * YAxis * 2 * ItemBoundExtend.Y * j;
					FVector RelativePositionOnZAxis = ZAxis * 2 * ItemBoundExtend.Z * s;

					FVector ItemPlacingLocation = StartVector + RelativePointOnXAxis + RelativePointOnYAxis + RelativePositionOnZAxis;

					if (bIsDebugMode)
					{
						DrawDebugPoint(GetWorld(), ItemPlacingLocation, 10, FColor::Green, false, 3);
					}

					Clone->SetActorLocation(ItemPlacingLocation);

					// Check for collisions
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
	if (ItemTemplate->ObjectInfo.bCanBeHookedUp == false) return; // The item we try to place on the hook is not a hookable item

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

	ItemTemplate->K2_SetActorRotation(PlaceItemRotation, true);// Set rotation before checking for extends

	FVector ItemOrigin;
	FVector ItemExtend;
	ItemTemplate->GetActorBounds(false, ItemOrigin, ItemExtend);

	ItemTemplate->SetActorRotation(Hit.GetComponent()->GetComponentRotation() + PlaceItemRotation); // Set item's rotation to blocking volume rotation + the item rotation

	// *** Calculate amount of items 
	int AmountOfItemsToPlace = 1;

	if (bRowModeKeyIsHeldDown) {
		AmountOfItemsToPlace = FMath::FloorToInt((2 * HookExtendOnX) / (2 * ItemExtend.X * (SpacingX + MINIMUM_GAP_BETWEEN_ITEMS)));
	}
	// *** *** *** ***

	FVector XAxis = Hit.GetComponent()->GetRightVector();
	FVector FrontItemPosition = CenterOfBox + HookExtendOnX * XAxis - HoleComponent->RelativeLocation; // The position at the front of the hook

	// Show the phantom items
	for (size_t i = 0; i < AmountOfItemsToPlace; i++)
	{
		FVector RelativePointOnXAxis = (MINIMUM_GAP_BETWEEN_ITEMS + SpacingX) * (-XAxis) * 2 * ItemExtend.X * i;

		ARRefillObject* PhantomItem = GetCloneActor(ItemTemplate);
		PhantomItems.Add(PhantomItem);
		PhantomItem->SetActorLocation(FrontItemPosition + RelativePointOnXAxis);

		// Check for collisions
		bItemCanBePlaced = CheckCollisions(PhantomItem);
	}

	LastFocussedHook = Hit.GetComponent();
}

void UCPlaceItem::SelectItem(ARRefillObject* Item)
{
	SelectedItems.Add(Item);
	Item->GetStaticMeshComponent()->SetRenderCustomDepth(true); // Highlight the items
}

void UCPlaceItem::SelectRow()
{
	if (ItemManager != nullptr) {
		UActorComponent* HookComponent = ItemManager->FindHookOfItem(FocussedItem);

		if (HookComponent != nullptr) {
			// It is an item on a hook
			if (ItemManager->ItemsOnHooks.Contains(HookComponent)) {
				DeselectItems();

				TArray<ARRefillObject*> AllItemsOnTheSameHook = ItemManager->ItemsOnHooks[HookComponent]; // Get all other items on the same hook

				for (auto& ItemOnHook : AllItemsOnTheSameHook) {
					SelectItem(ItemOnHook); // Seletct all items on the same hook
				}

				UE_LOG(LogTemp, Log, TEXT("%s: Found %i hooked items"), *FString(__FUNCTION__), SelectedItems.Num());
			}
		}
		else {
			// It is a shelf item
			ABlockingVolume* BlockingVol = FindShelfOfFocussedActor(); // Get the shelf the focused item is standing on

			if (BlockingVol == nullptr)
			{
				if (SelectedItems.Contains(FocussedItem) == false)
				{
					SelectedItems.Add(FocussedItem); // There is no blocking volume. Select only focused item
				}

				return; // That wasn't a blocking volume we hit 
			}

			TArray<FHitResult> Hits;

			// Start point is the point of the focused item
			FVector StartVectorTemp = GetRowStartPoint(BlockingVol, FocussedItem, FocussedItem->GetActorLocation());
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
				ARRefillObject* HitActor = Cast<ARRefillObject>(elem.GetActor());

				if (HitActor != nullptr && ItemManager->ListOfRefillTitemsPlacedInWorld.Contains(HitActor))
				{
					CheckStackedNeighbour(HitActor); // Check if this item has any other item stacked onto it
					SelectItem(HitActor); // Select each single item
				}
			}

			if (bIsDebugMode)
			{
				DrawDebugLine(GetWorld(), StartVector, EndVector, FColor::Blue, false, 3, 5);
			}
		}
	}
}

void UCPlaceItem::SelectAllItems()
{
	ABlockingVolume* BlockingVol = FindShelfOfFocussedActor(); // Find the shelf the item is standing on

	if (BlockingVol == nullptr)
	{
		if (SelectedItems.Contains(FocussedItem) == false)
		{
			SelectedItems.Add(FocussedItem); // Only select focused item
		}

		return; // That wasn't a blocking volume we hit
	}

	TArray<AActor*> ItemsOnShelf;
	BlockingVol->GetOverlappingActors(ItemsOnShelf); // Get all the actors touching the blocking volume

	for (auto & HitActor : ItemsOnShelf)
	{
		ARRefillObject* CastedActor = Cast<ARRefillObject>(HitActor);
		if (CastedActor != nullptr && ItemManager->ListOfRefillTitemsPlacedInWorld.Contains(CastedActor))
		{
			CheckStackedNeighbour(CastedActor); // Check if this item has any other item stacked onto it
			SelectItem(CastedActor); // Select each single item
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
	if (FocussedItem != nullptr && bRowModeKeyIsHeldDown == false && bWholeShelfModeKeyIsHeldDown == false)
	{
		GetStaticMesh(FocussedItem)->SetRenderCustomDepth(true);
		SelectedItems.Add(FocussedItem);
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
	PlaceItemRotation += FRotator(0, 90, 0);
	bInputHasBeenChanged = true;
}

void UCPlaceItem::TogglePlacingRemovingState()
{
	if (CurrentInteractionState == InteractionState::PLACING)
	{
		CurrentInteractionState = InteractionState::SELECTING;
		DeactivatePhantomItems(); // Deactivate all PhantomItems when switching to selection mode

		GEngine->AddOnScreenDebugMessage(TextID_PlacingSelectingMode, 1000000.0f, FColor::Blue, "Current Mode: SELECTING", false);
	}
	else
	{
		CurrentInteractionState = InteractionState::PLACING;
		FocussedItem = nullptr; // We don't focuss anything anymore
		GEngine->AddOnScreenDebugMessage(TextID_PlacingSelectingMode, 1000000.0f, FColor::Blue, "Current Mode: PLACING", false);
	}
}

ARRefillObject* UCPlaceItem::GetCloneActor(ARRefillObject* ActorToClone)
{
	for (auto& object : ObjectPool) // First check the object pool if there is any unused object
	{
		if (object->bHidden) // Take the first object that is hidden (= unused)
		{
			// Reaactivate the object
			object->SetActorHiddenInGame(false);
			object->SetActorEnableCollision(true);

			UStaticMeshComponent* MeshComponent = GetStaticMesh(object);
			UStaticMesh* MeshOfActorToClone = GetStaticMesh(ActorToClone)->GetStaticMesh();

			// Setup the Mesh
			if (MeshComponent->GetStaticMesh() != MeshOfActorToClone)
			{
				MeshComponent->SetStaticMesh(MeshOfActorToClone);
			}

			// ... and material
			MeshComponent->SetMaterial(0, ItemMaterial);

			// Enable physics and collision
			MeshComponent->SetCollisionProfileName("OverlapAll");
			MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			MeshComponent->SetSimulatePhysics(false);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			MeshComponent->bGenerateOverlapEvents = true;

			object->K2_SetActorRotation(ItemTemplate->GetActorRotation(), false);

			// Add the object information
			object->ObjectInfo = ActorToClone->ObjectInfo;

			return object;
		}
	}

	// If we came here, we don't have an unused objects in the object pool. We create a new one
	FActorSpawnParameters Parameters;
	Parameters.Template = ActorToClone;

	ARRefillObject* Clone = GetWorld()->SpawnActor<ARRefillObject>(ARRefillObject::StaticClass(), Parameters);
	Clone->ObjectInfo = ActorToClone->ObjectInfo;

	UStaticMeshComponent* MeshComponent = GetStaticMesh(Clone);

	ItemMaterial = MeshComponent->GetMaterial(0);
	MeshComponent->SetCollisionProfileName("OverlapAll"); // A phantom item shouldn't collide with anything but overlap
	MeshComponent->bGenerateOverlapEvents = true;

	ObjectPool.Add(Clone);

	return Clone;
}

bool UCPlaceItem::CheckCollisions(AActor * Actor, UActorComponent* IgnoredComponent)
{
	UStaticMeshComponent* ActorMesh = GetStaticMesh(Actor);

	bool bActorPlacable = true;
	GetStaticMesh(Actor)->bGenerateOverlapEvents = true; // Make sure the item is generating overlap events

	TArray<UPrimitiveComponent*> HitComponents;
	Actor->GetOverlappingComponents(HitComponents); // Get all overlapping components

	for (auto& elem : HitComponents) {
		if (ItemManager != nullptr && ItemManager->SetOfHooks.Contains(elem) == false && SetOfShelves.Contains(elem->GetOwner()) == false && PhantomItems.Contains(elem->GetOwner()) == false) {
			// It's not a shelf or hook and not another phantom item
			UE_LOG(LogTemp, Log, TEXT("%s: Overlapping with %s of actor %s"), *FString(__FUNCTION__), *elem->GetName(), *elem->GetOwner()->GetName());
			bActorPlacable = false;
			break;
		}
	}

	if (bActorPlacable == false)
	{
		// Apply the red material to every phantom object
		if (CollisionMaterial != nullptr)
		{
			for (auto &elem : PhantomItems)
			{
				// Color it red
				UStaticMeshComponent* Mesh = GetStaticMesh(elem);

				if (Mesh != nullptr)
				{
					Mesh->SetMaterial(0, CollisionMaterial);
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
		ItemMaterial = GetStaticMesh(SpawnedActor)->GetMaterial(0); // Get the material of the newly spawned item
	}

	ItemTemplate = Cast<ARRefillObject>(SpawnedActor); // Assign the newly spawned actor as the new ItemTemplate

	if (ItemTemplate != nullptr)
	{
		GetStaticMesh(ItemTemplate)->SetMaterial(0, ItemMaterial); // Set the material of the new ItemTemplate to the material of the newly spawned item

		if (CurrentInteractionState == InteractionState::SELECTING) TogglePlacingRemovingState(); // Switch to PlacingMode if neccessary
	}

	ObjectPool.Empty(); // Clear out the object pool
}

FVector UCPlaceItem::GetRowStartPoint(ABlockingVolume* BlockingVolume, AActor* ItemToStartFrom, FVector ImpactPoint)
{
	// Calculate an intersection point between a line from the impact point to the lower (relative) x axis of the blocking volume
	FVector BlockingVolumeExtend = BlockingVolume->Brush->Bounds.BoxExtent;
	FVector XAxis = BlockingVolume->GetActorForwardVector();
	FVector YAxis = BlockingVolume->GetActorRightVector();
	FVector BlockingVolumeOrigin = BlockingVolume->GetActorLocation();

	// *** Item bounds ***
	FVector ItemOrigin;
	FVector ItemBoundExtend;
	ItemToStartFrom->GetActorBounds(false, ItemOrigin, ItemBoundExtend);
	// *** *** *** *** ***

	FVector SecondPointOfLine = ImpactPoint - BlockingVolumeExtend.X * XAxis;
	FVector BasePointPlane = BlockingVolumeOrigin - BlockingVolumeExtend.X * XAxis - BlockingVolumeExtend.Y * YAxis; // The position vector of the plane
	FVector NormalVectorOfPlane = XAxis;

	// The point of intersection between the line and the plane
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
	// *** *** *** *** *** *** *** *** *** *** *** 

	for (const auto& elem : StackHits)
	{
		ARRefillObject* StackedActor = Cast<ARRefillObject>(elem.GetActor());
		if (StackedActor != nullptr && ItemManager->ListOfRefillTitemsPlacedInWorld.Contains(StackedActor))
		{
			if (SelectedItems.Contains(StackedActor) == false) // Check if we already selected this item
			{
				SelectItem(StackedActor);
				CheckStackedNeighbour(StackedActor); // Call recursivly
			}
		}
	}
}

ABlockingVolume * UCPlaceItem::FindShelfOfFocussedActor()
{
	ABlockingVolume* BlockingVol = nullptr;

	TArray<FHitResult> BlockingVolumeHits;
	FCollisionQueryParams TraceParams;
	TraceParams.TraceTag = FName(TAG_SHELF);
	TraceParams.AddIgnoredActor(FocussedItem);

	FVector StartVector = FocussedItem->GetActorLocation();
	FVector EndVector = StartVector - FocussedItem->GetActorUpVector() * 100.0f; // Quite long vector

	// Raycast down to find the blocking volume the item is standing in
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
	if (FocussedItem == nullptr)
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
	bRowModeKeyWasPressed = true;
	bRowModeKeyIsHeldDown = true;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::OnKeyRowModeReleased()
{
	bRowModeKeyWasReleased = true;
	bRowModeKeyIsHeldDown = false;
	bInputHasBeenChanged = true;

	DeselectItems();
}

void UCPlaceItem::OnKeyWholeShelfModePressed()
{
	bWholeShelfModeKeyWasPressed = true;
	bWholeShelfModeKeyIsHeldDown = true;
	bInputHasBeenChanged = true;
}

void UCPlaceItem::OnKeyWholeShelfModeReleased()
{
	bWholeShelfModeKeyWasReleased = true;
	bWholeShelfModeKeyIsHeldDown = false;
	bInputHasBeenChanged = true;

	DeselectItems();
}

void UCPlaceItem::ResetInputKeys() {
	bRowModeKeyWasPressed = false;
	bRowModeKeyWasReleased = false;

	bWholeShelfModeKeyWasPressed = false;
	bWholeShelfModeKeyWasReleased = false;

	bInputHasBeenChanged = false;
}


