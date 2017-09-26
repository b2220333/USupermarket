// Custom HUD

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
class REFILL_API AMyHUD : public AHUD
{
	GENERATED_BODY()
		AMyHUD();

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The font
	UPROPERTY()
		UFont* HUDFont;
	
	// Crosshair texture
	UPROPERTY()
		UTexture2D* Crosshair;

	// The maximim size of the preview image for items
	UPROPERTY(EditAnywhere)
		float ItemImageDimension; // The maximum height and width of the item image

	// The widged instance for the list of items
	UPROPERTY()
	UHUDWidget* WidgetInstance;

	virtual void DrawHUD() override;

	// Shows the list of available items
	void ShowListWidget(); 

private:
	// The preview image of an item
	UPROPERTY()
		UTexture2D* ItemImage;

	// Image width
	int32 ImageWidth;
	// Image height
	int32 ImageHeight;

	// The items name
	FString SelectedItemName;

	bool bListIsVisible;
	bool bCrosshairTextureIsValid;

	// Reads an image 
	static EImageFormat::Type GetJoyImageFormat(EJoyImageFormats JoyFormat);

	// Loads an texture from path
	UTexture2D* LoadTexture2D_FromFile(const FString& FullFilePath, EJoyImageFormats ImageFormat, bool& IsValid, int32& Width, int32& Height);

	
	// *** UNUSED ***
	static FString GetJoyImageExtension(EJoyImageFormats JoyFormat);


public:

	/**
	* Displays the image and the name of the item
	* @param ImagePath	The path to the image of the item
	* @param ItemName	The name of the item
	*/
	void PreviewRefillItem(FString ImagePath, FString ItemName);

	void DisablePreviewImage();

};
