// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "MyHUD.h"

AMyHUD::AMyHUD() {

	// Other fonts can be found at C:\Program Files (x86)\Epic Games\UE_4.15\Engine\Content\EngineFonts
	ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Engine/EngineFonts/DroidSansMono"));

	if (FontObject.Object)
	{
		HUDFont = FontObject.Object;
	}

	HUDTextCounter = 0;
}

void AMyHUD::BeginPlay()
{
	FOnActorSpawned::FDelegate ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &AMyHUD::OnActorSpawned);
	GetWorld()->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
}

void AMyHUD::DrawHUD()
{
	Super::DrawHUD();

	int	CanvasWidth = Canvas->SizeX;
	int CanvasHeight = Canvas->SizeY;
	float LineSpacing = 0;

	FString NewString;

	for (auto &Text : HUDTexts)
	{
		NewString.Append(Text.Value);
		NewString.Append("\n");
	}

	DrawText(NewString, FLinearColor::White, CanvasWidth - 300, 10);
}

int AMyHUD::AppendText(const FString Text)
{
	HUDTextCounter++;
	HUDTexts.Add(HUDTextCounter, Text);

	return HUDTextCounter;
}

void AMyHUD::ChangeText(const int TextNumber, const FString NewText)
{
	if (HUDTexts.Contains(TextNumber))
	{
		HUDTexts[TextNumber] = NewText;
	}
}

void AMyHUD::RemoveText(const int TextNumber)
{
	if (HUDTexts.Contains(TextNumber))
	{
		HUDTexts.Remove(TextNumber);
	}
}

void AMyHUD::OnActorSpawned(AActor* SpawnedActor)
{
	// TODO Check if this actor implements the interface to get its stats
	//UE_LOG(LogTemp, Warning, TEXT("AMyHUD::OnActorSpawned: Actor %s spawned"), *SpawnedActor->GetName());

	// TODO The problem here is that we spawn clones of the object everytime we move the mouse cursor which will then fire the callback a lot
}



