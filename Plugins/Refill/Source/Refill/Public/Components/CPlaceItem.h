// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include "Components/ActorComponent.h"
#include "HoleTabComponent.h"
// #include "Structs/Obb.h"
#include "DrawDebugHelpers.h"
#include "../AssetLoader/RAssetLoader.h"
#include "../AssetLoader/CacheAssetLoader.h"
#include "MyHUD.h"
#include "CPlaceItem.generated.h"

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

	UPROPERTY(EditAnywhere)
		bool bIsDebugMode;

	UPROPERTY(EditAnywhere)
		float RaycastRange;

	// The spacing along the X-axis
	UPROPERTY(EditAnywhere)
		float SpacingX;

	// The spacing along the Y-axis
	UPROPERTY(EditAnywhere)
		float SpacingY;

	UPROPERTY(EditAnywhere)
		int Stacking;

	//UPROPERTY(EditAnywhere)
	//	ARAssetLoader* AssetLoader;

	UPROPERTY(EditAnywhere)
		ACacheAssetLoader* AssetLoader;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetupKeyBindings(UInputComponent* PlayerInputComponent);

	ACharacter* Character;

	// Material for the object when it is colliding
	UPROPERTY(EditAnywhere)
		UMaterialInterface* RedMaterial;

	UPROPERTY()
		UMaterialInterface* ItemMaterial;

	TArray<AActor*> ListOfRefillTitemsPlacedInWorld;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/*std::pair<FHitResult, TArray<FName>>*/ FHitResult StartRaytrace();

	// Inreases or decreases the space between the items
	void ChangeSpacing(const float Val);

	// Places the items as shown by the phantom items
	void PlaceItems();

	// Removes all selected items
	void RemoveItems();

	// Reacts to the event when player presses left mouse button 
	void OnFirePressed();

private:

	//**** UI TextmessageIDs
	int TextID_XSpacing = 1;
	int TextID_YSpacing = 2;
	int TextID_SpacingStep = 3;
	int TextID_Error = 4;
	int TextID_PlacingSelectingMode = 5;
	int TextID_RowMode = 6;
	int TextID_WholeShelfMode = 7;
	//**********************

	AActor* ItemTemplate;

	FVector LocationToPlaceItem;
	FRotator Rotation;


	TSet<AActor*> SetOfShelves;
	TSet<UActorComponent*> SetOfHooks;
	UPrimitiveComponent* LastFocusedHook;

	TMap<FName, UActorComponent*> HooknamesToHookComponent;
	TMap<UActorComponent*, TArray<AActor*>> ItemsOnHook;

	// The current state (placing, removing) of the player
	InteractionState CurrentInteractionState;

	bool bItemCanBePlaced;

	// True if the player has chenged the position or rotation of the item
	bool bPlayerHasMoved;

	FHitResult RaytraceResults;


	// The value how much to change the gap between items in placing mode
	float SpacingStep;

	AMyHUD* HUD;

	// The item, the player is loocking at
	AActor* FocusedItem;

	// A list of all items selected
	TArray<AActor*> SelectedItems;

	// A list of reusable objects to avoid spawning and destroying objects each tick
	UPROPERTY()
		TArray<AActor*> ObjectPool;

	// The Items to show before placing them
	UPROPERTY()
		TArray<AActor*> PhantomItems;

	// Checks if the player has moved or pressed any button and makes new selections
	void CheckPlayerInput();

	// Displays the phantom item at a position where the raytrace hit the shelf
	void DisplayPhantomItem(FHitResult Hit);

	void DisplayPhantomHookItem(FHitResult Hit);

	void SelectItem(AActor* Item); // Selects and highlights an item
	void SelectRow(); // Selects all items in a row
	void SelectAllItems(); // Selects all items on a shelf
	void DeselectItems(); // Deselects all items

	void CreateConstraintsForHookableItems(AActor* Actor);

	// Helper function to get the static mesh from an actor
	UStaticMeshComponent* GetStaticMesh(AActor* Actor);



	// Returns a clone of the given actor
	AActor* GetCloneActor(AActor* ActorToClone);

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
	ABlockingVolume* FindShelf(AActor* FromActor);
	// Finds the hook the item is hanging on
	UActorComponent* FindHookOfItem(AActor* Item);

	void DeactivatePhantomItems();

	// *** INPUT ***
	bool bLeftShiftWasPressed;
	bool bLeftControlWasPressed;

	bool bLeftShiftIsHeldDown;
	bool bLeftControlIsHeldDown;

	bool bLeftShiftWasReleased;
	bool bLeftControlWasReleased;

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
};
