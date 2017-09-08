// Fill out your copyright notice in the Description page of Project Settings.

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


