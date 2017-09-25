// Handles mostly items on hooks

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "RRefillObject.h"
#include "ItemManager.generated.h"

UCLASS()
class REFILL_API AItemManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Adds an already spawned item to the hook
	void AddItemToHook(UActorComponent* HookComponent, ARRefillObject* ItemToHookUp);
	// Adds an already spawned item to the hook
	void AddItemToHook(UActorComponent* HookComponent, AStaticMeshActor* ItemToHookUp);
	// Adds an already spawned item to the hook
	void AddItemToHook(UActorComponent* HookComponent, AActor* ItemToHookUp);

	// Removes an item from the hook but doesn't destroy the actor
	void RemoveItemFromHook(UActorComponent* HookComponent, ARRefillObject* Item);
	
	// Returns the hook the given item is hanging on
	UActorComponent* FindHookOfItem(ARRefillObject* Item);

	// A list of all refill items in the world
	TArray<AActor*> ListOfRefillTitemsPlacedInWorld;

	// A set of all hooks in the world
	TSet<UActorComponent*> SetOfHooks;

	// A map of each hook with its items
	TMap<UActorComponent*, TArray<ARRefillObject*>> ItemsOnHooks;

	// A map from the hook's component name to the hook component
	TMap<FName, UActorComponent*> HooknamesToHookComponent;

private:
	// Adds an already spawned item to the hook
	void AddItemToHook(UPrimitiveComponent* HookComponent, ARRefillObject* ItemToHookUp);
};
