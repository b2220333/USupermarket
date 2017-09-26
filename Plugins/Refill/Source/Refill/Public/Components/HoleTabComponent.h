// A dummy component to indicate the hole of an item which can be placed on a hook

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HoleTabComponent.generated.h"

/**
 *
 */
UCLASS()
class REFILL_API UHoleTabComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	//UHoleTabComponent();
	UHoleTabComponent(const FObjectInitializer& ObjectInitializer);
};


