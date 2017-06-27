// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "HUDWidget.h"
#include "Runtime/ImageWrapper/Public/ImageWrapper.h"
#include "MyHUD.generated.h"

UENUM()
enum class EJoyImageFormats : uint8
{
	JPG		UMETA(DisplayName = "JPG        "),
	PNG		UMETA(DisplayName = "PNG        "),
	BMP		UMETA(DisplayName = "BMP        "),
	ICO		UMETA(DisplayName = "ICO        "),
	EXR		UMETA(DisplayName = "EXR        "),
	ICNS	UMETA(DisplayName = "ICNS        ")
};

/**
 *
 */
UCLASS()
class PLACINGOBJECTS_API AMyHUD : public AHUD
{
	GENERATED_BODY()
		AMyHUD();

public:
	//UPROPERTY(EditAnywhere,/*EditDefaultsOnly, BlueprintReadOnly,*/ Category = UI)
	//	TSubclassOf<UUserWidget> WidgetTemplate;

	//UPROPERTY()
	//	UUserWidget* WidgetInstance;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
		UFont* HUDFont;

	TMap<int, FString> HUDTexts;
	int HUDTextCounter;

	FString DisplayText;

	UPROPERTY()
		UTexture2D* Crosshair;

	UPROPERTY(EditAnywhere)
		float ItemImageDimension; // The maximum height and with of the item image
	
	UPROPERTY()
	UHUDWidget* WidgetInstance;

	virtual void DrawHUD() override;


	void OnActorSpawned(AActor* SpawnedActor);
	void ShowListWidget(); // Shows the list of available items

private:
	UPROPERTY()
		UTexture2D* ItemImage;
	int32 ImageWidth;
	int32 ImageHeight;
	FString SelectedItemName;

	bool bListIsVisible;


	static EImageFormat::Type GetJoyImageFormat(EJoyImageFormats JoyFormat);

	UTexture2D* LoadTexture2D_FromFile(const FString& FullFilePath, EJoyImageFormats ImageFormat, bool& IsValid, int32& Width, int32& Height);





	// *** UNUSED ***
	static FString GetJoyImageExtension(EJoyImageFormats JoyFormat);


public:

	/**
	* Displays the image and the name of the item
	* @param ImagePath	The path to the image of the item
	* @param ItemName	The name of the item
	*/
	void ShowRefillItem(FString ImagePath, FString ItemName);
};
