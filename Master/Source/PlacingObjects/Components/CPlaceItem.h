// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include "Components/ActorComponent.h"
#include "Components/HoleTabComponent.h"
#include "Structs/Obb.h"
#include "DrawDebugHelpers.h"
#include "AssetLoader/RAssetLoader.h"
#include "AssetLoader/CacheAssetLoader.h"
#include "MyHUD.h"
#include "CPlaceItem.generated.h"

UENUM()
enum class InteractionState : uint8
{
	PLACING		UMETA(DisplayName = "Placing State"),
	SELECTING	UMETA(DisplayName = "Selecting State")
};
 
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLACINGOBJECTS_API UCPlaceItem : public UActorComponent
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
	ACharacter* Character;

	// Material for the object when it is colliding
	UPROPERTY(EditAnywhere)
		UMaterialInterface* RedMaterial;

	UPROPERTY()
		UMaterialInterface* ItemMaterial;

	UPROPERTY(EditAnywhere)
		AActor* ItemTemplate;

	//UPROPERTY(EditAnywhere)
	//	FString ItemTag;

	//TArray<AActor*> GetListOfItems() {
	//	return ListOfRefillTitemsPlacedInWorld;
	//}

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
	int iTextID_XSpacing = 1;
	int iTextID_YSpacing = 2;
	int iTextID_SpacingStep = 3;
	int iTextID_Error = 4;
	//**********************

	FVector LocationToPlaceItem;
	FRotator Rotation;


	TSet<AActor*> SetOfShelves;
	TSet<UActorComponent*> SetOfHooks;
	UPrimitiveComponent* LastFocusedHook;

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

	void SelectRow(); // Selects all items in a row
	void SelectAllItems(); // Selects all items on a shelf
	void DeselectItems(); // Deselects all items

	void CreateConstraintsForHookableItems(AActor* Actor);

	// Helper function to get the static mesh from an actor
	UStaticMeshComponent* GetStaticMesh(AActor* Actor);

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
};
