// The object class for all items which can be placed in the world by the player

#pragma once

#include "Engine/StaticMeshActor.h"
#include "RefillObjectInfo.h"
#include "RRefillObject.generated.h"

UCLASS()
class REFILL_API ARRefillObject : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// The item name
	FString ItemName;

	// The information structure 
	FRefillObjectInfo ObjectInfo;

public:
	// Sets default values for this actor's properties
	ARRefillObject();

	ARRefillObject(const FObjectInitializer& PCIP);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Loads the meshes and materials for this item
	void LoadRefillObject(const FString AssetPath);
};
