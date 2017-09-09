#define CROSSHAIR_PATH "Images/crosshair.png"
#define NOIMAGE_PATH "Images/NoImage.png"
// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "HUDWidget.h"
#include "RMyCharacter.h"
#include "MyHUD.h"

AMyHUD::AMyHUD() {

	// Other fonts can be found at C:\Program Files (x86)\Epic Games\UE_4.15\Engine\Content\EngineFonts
	ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Engine/EngineFonts/DroidSansMono"));

	if (FontObject.Object)
	{
		HUDFont = FontObject.Object;
	}

	ItemImageDimension = 300;

	HUDTextCounter = 0;
}

void AMyHUD::BeginPlay()
{
	FOnActorSpawned::FDelegate ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &AMyHUD::OnActorSpawned);
	GetWorld()->AddOnActorSpawnedHandler(ActorSpawnedDelegate);

	int32 Width;
	int32 Height;
	Crosshair = LoadTexture2D_FromFile(FPaths::GameContentDir() + CROSSHAIR_PATH, EJoyImageFormats::PNG, bCrosshairTextureIsValid, Width, Height);

	if (bCrosshairTextureIsValid == false) {
		UE_LOG(LogTemp, Warning, TEXT("%s: Could not find crosshair image %s"), *FString(__FUNCTION__), CROSSHAIR_PATH);
	}

	ACharacter* Character = Cast<ARMyCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (Character)
	{
		UInputComponent* PlayerInputComponent = Character->InputComponent;

		if (PlayerInputComponent)
		{
			PlayerInputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AMyHUD::ShowListWidget); // TODO hardcoded key
		}
	}
}

void AMyHUD::ShowListWidget()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Toggling inventory"), *FString(__FUNCTION__));

	if (bListIsVisible)
	{
		// Disable list
		if (WidgetInstance != nullptr)
		{
			WidgetInstance->ConditionalBeginDestroy();
			bListIsVisible = false;

			FInputModeGameOnly Mode;
			GetWorld()->GetFirstPlayerController()->SetInputMode(Mode);
			GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false;
		}

		DisablePreviewImage();
	}
	else
	{
		// Enable list
		WidgetInstance = CreateWidget<UHUDWidget>(GetWorld(), UHUDWidget::StaticClass());

		if (WidgetInstance != nullptr)
		{
			WidgetInstance->AddToViewport();
			bListIsVisible = true;

			// Focus on list and activate cursor
			FInputModeGameAndUI Mode;
			Mode.SetHideCursorDuringCapture(false);
			GetWorld()->GetFirstPlayerController()->SetInputMode(Mode);
			GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;

			UE_LOG(LogTemp, Log, TEXT("%s: ShowListWidget: Widget added to viewport"), *FString(__FUNCTION__));
		}
	}
}

void AMyHUD::DrawHUD()
{
	Super::DrawHUD();

	int	CanvasWidth = Canvas->SizeX;
	int CanvasHeight = Canvas->SizeY;
	float LineSpacing = 0;

	DrawText(SelectedItemName, FLinearColor::Blue, 10, CanvasHeight - 30);

	if (bCrosshairTextureIsValid) {
		// Draws crosshair
		float ScaleFactorCrosshair = 0.2f;
		float PosX = Canvas->SizeX / 2 - Crosshair->GetSizeX() / 2 * ScaleFactorCrosshair;
		float PosY = Canvas->SizeY / 2 - Crosshair->GetSizeY() / 2 * ScaleFactorCrosshair;
		DrawTextureSimple(Crosshair, PosX, PosY, ScaleFactorCrosshair);
	} 

	// Draws item image
	if (ItemImage != nullptr)
	{
		float ScalefactorImage = ItemImageDimension / ((ItemImage->GetSizeX() + ItemImage->GetSizeY()) / 2);
		float PosX = Canvas->SizeX / 2 - ItemImage->GetSizeX() / 2 * ScalefactorImage;
		float PosY = Canvas->SizeY / 2 - ItemImage->GetSizeY() / 2 * ScalefactorImage;

		//DrawTextureSimple(ItemImage, 0, Canvas->SizeY - 50, ScalefactorImage);
		DrawTextureSimple(ItemImage, PosX, PosY, ScalefactorImage); // Display image in center
	}
}

void AMyHUD::PreviewRefillItem(FString ImagePath, FString ItemName) {
	bool IsValid;

	ItemImage = LoadTexture2D_FromFile(ImagePath, EJoyImageFormats::PNG, IsValid, ImageWidth, ImageHeight);

	if (IsValid == false) {
		UE_LOG(LogTemp, Warning, TEXT("%s: Item image (%s) not found"), *FString(__FUNCTION__), *ImagePath.Append(ItemName));
	
		ItemImage = LoadTexture2D_FromFile(FPaths::GameContentDir() + NOIMAGE_PATH, EJoyImageFormats::PNG, IsValid, ImageWidth, ImageHeight);

		if(IsValid == false) UE_LOG(LogTemp, Warning, TEXT("%s: NoImage image couldn't be found"), *FString(__FUNCTION__));

	}



}

void AMyHUD::DisablePreviewImage()
{
	ItemImage = nullptr;
}


FString AMyHUD::GetJoyImageExtension(EJoyImageFormats JoyFormat)
{

	switch (JoyFormat)
	{
	case EJoyImageFormats::JPG: return ".jpg";
	case EJoyImageFormats::PNG: return ".png";
	case EJoyImageFormats::BMP: return ".bmp";
	case EJoyImageFormats::ICO: return ".ico";
	case EJoyImageFormats::EXR: return ".exr";
	case EJoyImageFormats::ICNS: return ".icns";
	}
	return ".png";

}

EImageFormat::Type AMyHUD::GetJoyImageFormat(EJoyImageFormats JoyFormat)
{
	switch (JoyFormat)
	{
	case EJoyImageFormats::JPG: return EImageFormat::JPEG;
	case EJoyImageFormats::PNG: return EImageFormat::PNG;
	case EJoyImageFormats::BMP: return EImageFormat::BMP;
	case EJoyImageFormats::ICO: return EImageFormat::ICO;
	case EJoyImageFormats::EXR: return EImageFormat::EXR;
	case EJoyImageFormats::ICNS: return EImageFormat::ICNS;
	}
	return EImageFormat::JPEG;
}



UTexture2D* AMyHUD::LoadTexture2D_FromFile(const FString& FullFilePath, EJoyImageFormats ImageFormat, bool& IsValid, int32& Width, int32& Height)
{
	IsValid = false;
	UTexture2D* LoadedT2D = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(GetJoyImageFormat(ImageFormat));

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath)) return NULL;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D) return NULL;
			//~~~~~~~~~~~~~~

			//Out!
			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();

			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}

	// Success!
	IsValid = true;
	return LoadedT2D;
}



void AMyHUD::OnActorSpawned(AActor* SpawnedActor)
{
	// TODO Check if this actor implements the interface to get its stats
	// UE_LOG(LogTemp, Warning, TEXT("AMyHUD::OnActorSpawned: Actor %s spawned"), *SpawnedActor->GetName());

	// TODO The problem here is that we spawn clones of the object everytime we move the mouse cursor which will then fire the callback a lot
}



