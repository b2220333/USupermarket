// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include "Runtime/UMG/Public/UMG.h"
// #include "Slate.h"
#include "GameFramework/Character.h"
#include "RMyCharacter.generated.h"

UCLASS()
class PLACINGOBJECTS_API ARMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMyCharacter();
	
	UPROPERTY(EditAnywhere, Category = UI)
		TSubclassOf<UUserWidget> WidgetTemplate; // The template of to HUD widged to display the list of items

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


};
