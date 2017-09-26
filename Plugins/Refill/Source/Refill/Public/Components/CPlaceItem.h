// Handles picking up and selecting items

#pragma once

#include <utility>
#include "Components/ActorComponent.h"
#include "HoleTabComponent.h"
#include "ItemManager.h"
#include "DrawDebugHelpers.h"
#include "../AssetLoader/RAssetLoader.h"
#include "../AssetLoader/CacheAssetLoader.h"
#include "MyHUD.h"
#include "CPlaceItem.generated.h"

// The state the player can be in. Either selecting items or placing new items
UENUM()
enum class InteractionState : uint8
{
	PLACING		UMETA(DisplayName = "Placing State"),
	SELECTING	UMETA(DisplayName = "Selecting State")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class REFILL_API UCPlaceItem : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCPlaceItem();

	// Enables more debug information
	UPROPERTY(EditAnywhere, Category = "Refills")
		bool bIsDebugMode;

	// The raycast range of the player
	UPROPERTY(EditAnywhere, Category = "Refills")
		float RaycastRange;

	// The AssetLoader instance
	UPROPERTY(EditAnywhere, Category = "Refills")
		ACacheAssetLoader* AssetLoader;

	// Material for the object when it is colliding
	UPROPERTY(EditAnywhere, Category = "Refills")
		UMaterialInterface* CollisionMaterial;

	// The spacing along the X-axis
	float SpacingX;

	// The spacing along the Y-axis
	float SpacingY;

	// The stack height
	int Stacking;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Setup for key bindings
	void SetupKeyBindings(UInputComponent* PlayerInputComponent);

	// The player character instance
	ACharacter* Character;

	// The materials of the currently used ItemTemplate
	UMaterialInterface* ItemMaterial;

	// The item which the player can place
	ARRefillObject* ItemTemplate;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Raytrace function
	FHitResult StartRaytrace();

	// Inreases or decreases the space between the items
	void ChangeSpacing(const float Val);

	// Places the items as shown by the phantom items
	void PlaceItems();

	// Removes all selected items
	void RemoveItems();

	// Reacts to the event when player presses left mouse button 
	void OnFirePressed();

private:

	//**** GEngine UI Textmessage IDs
	int TextID_XSpacing = 1;
	int TextID_YSpacing = 2;
	int TextID_SpacingStep = 3;
	int TextID_Error = 4;
	int TextID_PlacingSelectingMode = 5;
	int TextID_RowMode = 6;
	int TextID_WholeShelfMode = 7;
	//**********************

	// The ItemManager instance
	AItemManager* ItemManager;

	// The HUD instance
	AMyHUD* HUD;

	// The current position the player is trying to place an item
	FVector PlaceItemLocation;
	// The item's roation
	FRotator PlaceItemRotation;

	// A set of all shelves (with the right TAG) in the world
	TSet<AActor*> SetOfShelves;

	// The last hook the player focussed
	UActorComponent* LastFocussedHook;

	// The current state (placing, removing) of the player
	InteractionState CurrentInteractionState;

	// Whether or not the current item can be placed
	bool bItemCanBePlaced;

	// The results of the last raytrace
	FHitResult RaytraceResults;

	// The value how much to change the gap between items in placing mode
	float SpacingStep;

	// The item, the player is loocking at
	ARRefillObject* FocussedItem;

	// A list of all items selected
	TArray<ARRefillObject*> SelectedItems;

	// A list of reusable objects to avoid spawning and destroying objects each tick
	UPROPERTY()
		TArray<ARRefillObject*> ObjectPool;

	// The Items to show before placing them
	UPROPERTY()
		TArray<ARRefillObject*> PhantomItems;

	// Checks if the player has moved or pressed any button and makes new selections
	void CheckPlayerInput();

	// Displays the phantom item at a position where the raytrace hit the shelf
	void DisplayPhantomItem(FHitResult Hit);

	// Shows the items on the hook the player is pointing at
	void DisplayPhantomHookItem(FHitResult Hit);

	void SelectItem(ARRefillObject* Item); // Selects and highlights an item
	void SelectRow(); // Selects all items in a row
	void SelectAllItems(); // Selects all items on a shelf
	void DeselectItems(); // Deselects all items

	// Helper function to get the static mesh from an actor
	UStaticMeshComponent* GetStaticMesh(AActor* Actor);

	// Returns a clone of the given actor
	ARRefillObject* GetCloneActor(ARRefillObject* ActorToClone);

	// Checks for collisions. Returns true if the item is placeable
	bool CheckCollisions(AActor* Actor, UActorComponent* IgnoredComponent = nullptr);

	// Gets called if a new item has been selected from the list of available items
	UFUNCTION()
		void OnNewItemSpawned(AActor* NewItem);

	// Returns the front point of a row
	FVector GetRowStartPoint(ABlockingVolume* BlockingVolume, AActor* ItemToStartFrom, FVector ImpactPoint);

	// In selection mode adds items that are stacked onto each other
	void CheckStackedNeighbour(AActor* FromActor);

	// Finds the shelf the actor is standing on
	ABlockingVolume* FindShelfOfFocussedActor();

	// Deactivates all PhantomItems
	void DeactivatePhantomItems();

	// *** INPUT ***
	bool bRowModeKeyWasPressed;
	bool bWholeShelfModeKeyWasPressed;

	bool bRowModeKeyIsHeldDown;
	bool bWholeShelfModeKeyIsHeldDown;

	bool bRowModeKeyWasReleased;
	bool bWholeShelfModeKeyWasReleased;

	bool bInputHasBeenChanged;


	// Increases the steps for spacing the items
	void IncreaseSpacingStep();

	// Decreases the steps for spacing the items
	void DecreaseSpacingStep();

	// Increases stack size
	void IncreaseStacking();

	// Decreases stack size
	void DecreaseStacking();

	// Steps the rotation of the item in 90 degree steps
	void StepRotation();

	// Toggles the state between removing and placing objects
	void TogglePlacingRemovingState();

	// Key for selecting a row
	void OnKeyRowModePressed();
	void OnKeyRowModeReleased();

	// Key input for selection all items on a shelf
	void OnKeyWholeShelfModePressed();
	void OnKeyWholeShelfModeReleased();

	void ResetInputKeys();
	// ***********************************************
};
