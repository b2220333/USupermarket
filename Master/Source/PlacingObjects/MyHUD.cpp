// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
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

	ItemImageDimension = 200;

	HUDTextCounter = 0;
}

//void AMyHUD::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//	WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetTemplate);
//	check(WidgetInstance != nullptr);
//}

void AMyHUD::BeginPlay()
{
	FOnActorSpawned::FDelegate ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &AMyHUD::OnActorSpawned);
	GetWorld()->AddOnActorSpawnedHandler(ActorSpawnedDelegate);

	bool IsValid;
	int32 Width;
	int32 Height;
	Crosshair = LoadTexture2D_FromFile(FPaths::GameContentDir() + "Images/crosshair.png", EJoyImageFormats::PNG, IsValid, Width, Height);

	
	//ARMyCharacter* Character = Cast<ARMyCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	//if (Character != nullptr)
	//{
	//	WidgetTemplate = Character->WidgetTemplate;
	//}

	//// Setting up widget for item list
	//if (WidgetTemplate != nullptr && WidgetInstance == nullptr)
	//{
	//	WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetTemplate);
	//	UE_LOG(LogTemp, Warning, TEXT("AMyHUD::BeginPlay: HUD instance created"));

	//	// TODO for testing
	//	UHUDWidget* Test = Cast<UHUDWidget>(WidgetInstance);
	//	if (Test != nullptr)
	//	{
	//		Test->ItemMap.Add(FString("Test 1"), nullptr);
	//		Test->ItemMap.Add(FString("Test 2"), nullptr);
	//		Test->ItemMap.Add(FString("Test 3"), nullptr);
	//		Test->ItemMap.Add(FString("Test 4"), nullptr);
	//		Test->ItemMap.Add(FString("Test 5"), nullptr);
	//	}
	//}

	//if (WidgetInstance != nullptr && !WidgetInstance->GetIsVisible())
	//{
	//	AMyHUD* HUD = Cast<AMyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	//	WidgetInstance->AddToViewport();
	//	UE_LOG(LogTemp, Warning, TEXT("AMyHUD::BeginPlay: Widget added to viewport"));
	//}	

	ACharacter* Character = Cast<ARMyCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (Character)
	{
		UInputComponent* PlayerInputComponent = Character->InputComponent;

		if (PlayerInputComponent)
		{
			PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AMyHUD::ShowListWidget); // TODO hardcoded key
		}
	}

	
}

void AMyHUD::ShowListWidget()
{
	if (bListIsVisible)
	{
		if (WidgetInstance != nullptr)
		{
			WidgetInstance->ConditionalBeginDestroy();
			bListIsVisible = false;

			FInputModeGameOnly Mode;
			GetWorld()->GetFirstPlayerController()->SetInputMode(Mode);	
			GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false;
		}
	}
	else
	{
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

			UE_LOG(LogTemp, Warning, TEXT("ShowListWidget: Widget added to viewport"));
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

	// Draws crosshair
	float ScaleFactorCrosshair = 0.2f;
	float PosX = Canvas->SizeX / 2 - Crosshair->GetSizeX() / 2 * ScaleFactorCrosshair;
	float PosY = Canvas->SizeY / 2 - Crosshair->GetSizeY() / 2 * ScaleFactorCrosshair;
	DrawTextureSimple(Crosshair, PosX, PosY, ScaleFactorCrosshair);

	// Draws item image
	if (ItemImage != nullptr)
	{
		float ScalefactorImage = ItemImage->GetSizeX() / ItemImageDimension;

		DrawTextureSimple(ItemImage, 0, Canvas->SizeY - 50, ScalefactorImage);
	}
}

void AMyHUD::ShowRefillItem(FString ImagePath, FString ItemName) {
	bool IsValid;

	ItemImage = LoadTexture2D_FromFile(ImagePath, EJoyImageFormats::PNG, IsValid, ImageWidth, ImageHeight);
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



