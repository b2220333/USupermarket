// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include "Components/ActorComponent.h"
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

	UPROPERTY(EditAnywhere)
		UMaterialInterface* RedMaterial;

	UPROPERTY()
		UMaterialInterface* ItemMaterial;

	UPROPERTY(EditAnywhere)
		AActor* ItemTemplate;

	UPROPERTY(EditAnywhere)
		FName ItemTag;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	std::pair<FHitResult, TArray<FName>> StartRaytrace();

	// Inreases or decreases the space between the items
	void ChangeSpacing(const float Val);

	// Places the items as shown by the phantom items
	void PlaceItems();

	void RemoveItems();

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

	InteractionState CurrentInteractionState;

	bool bItemCanBePlaced;

	// True if the player has chenged the position or rotation of the item
	bool bPlayerHasMoved;

	std::pair<FHitResult, TArray<FName>> RaytraceResults;

	float SpacingStep;

	AMyHUD* HUD;

	AActor* FocusedItem; // The item, the player is loocking at
	TArray<AActor*> SelectedItems; // A list of all items selected

	// A list of reusable objects to avoid spawning and destroying objects each tick
	UPROPERTY()
		TArray<AActor*> ObjectPool;

	// The Items to show before placing them
	UPROPERTY()
		TArray<AActor*> PhantomItems;
	
	void CheckPlayerInput(); // Checks if the player has moved or pressed any button and makes new selections

	// Displays the phantom item at a position where the raytrace hit the shelf
	void DisplayPhantomItem(FHitResult Hit);

	void SelectRow(); // Selects all items in a row
	void SelectAllItems(); // Selects all items on a shelf
	void DeselectItems();

	// Helper function to get the static mesh from an actor
	UStaticMeshComponent* GetStaticMesh(AActor* Actor);

	// Increases the steps for spacing the items
	void IncreaseSpacingStep();

	// Decreases the steps for spacing the items
	void DecreaseSpacingStep();

	void IncreaseStacking();

	void DecreaseStacking();

	// Steps the rotation of the item in 90 degree steps
	void StepRotation();

	// Toggles the state between removing and placing objects
	void TogglePlacingRemovingState();

	// Returns a clone of the given actor
	AActor* GetCloneActor(AActor* ActorToClone);

	// Checks for collisions
	bool CheckCollisions(AActor* Actor);

	void OnNewItemSelected();

	FVector GetRowStartPoint(ABlockingVolume* BlockingVolume, AActor* ItemToStartFrom, FVector ImpactPoint);

	// In selection mode adds items that are stacked onto each other
	void CheckStackedNeighbour(AActor* FromActor);

	ABlockingVolume* FindShelf(AActor* FromActor); // Finds the shelf the actor is standing on
};
