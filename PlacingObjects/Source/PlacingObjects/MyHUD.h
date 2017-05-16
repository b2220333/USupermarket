// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

/**
 * 
 */
UCLASS()
class PLACINGOBJECTS_API AMyHUD : public AHUD
{
	GENERATED_BODY()
		AMyHUD();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

		UPROPERTY()
		UFont* HUDFont;	

		TMap<int, FString> HUDTexts;
		int HUDTextCounter;

		FString DisplayText;

	virtual void DrawHUD() override;


	void OnActorSpawned(AActor* SpawnedActor);

public:

	int AppendText(const FString Text);

	void ChangeText(const int TextNumber, const FString NewText);

	void RemoveText(const int TextNumber);

};
