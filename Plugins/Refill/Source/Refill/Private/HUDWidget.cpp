// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "HUDWidget.h"

UHUDWidget::UHUDWidget(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Declare a font
	ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Engine/EngineFonts/DroidSansMono"));
	ButtonFont = FontObject.Object;
}

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

TSharedRef<SWidget> UHUDWidget::RebuildWidget()
{
	// Find AssetLoader
	if (AssetLoader == nullptr)
	{
		for (TObjectIterator<ACacheAssetLoader> Itr; Itr; ++Itr)
		{
			AssetLoader = *Itr;
		}
	}

	check(AssetLoader); // Assert AssetLoader

	// https://answers.unrealengine.com/storage/attachments/103428-capture.png
	UPanelWidget* RootWidget = Cast<UPanelWidget>(GetRootWidget());

	if (RootWidget == nullptr)
	{
		RootWidget = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootWidget"));

		UCanvasPanelSlot* RootWidgetSlot = Cast<UCanvasPanelSlot>(RootWidget->Slot);

		if (RootWidgetSlot)
		{
			// Set position and size of the list panel
			RootWidgetSlot->SetAnchors(FAnchors(0, 0, 1, 1));
			RootWidgetSlot->SetOffsets(FMargin(0, 0));
		}

		WidgetTree->RootWidget = RootWidget;
	}

	TSharedRef<SWidget> Widget = Super::RebuildWidget();

	if (RootWidget && WidgetTree)
	{
		// Add a scroll box
		UScrollBox* Scrollbox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Scroll list"));
		RootWidget->AddChild(Scrollbox);

		// Add a canvas to the scroll box
		UCanvasPanelSlot* ScrollboxSlot = Cast<UCanvasPanelSlot>(Scrollbox->Slot);

		if (ScrollboxSlot)
		{
			// ScrollboxSlot->SetAnchors(FAnchors(0.9f, 0, 1, 1)); // 0.9 means it always takes 10% of the screen
			ScrollboxSlot->SetAnchors(FAnchors(0.75f, 0, 1, 1)); // Set position of the scroll box canvas
			ScrollboxSlot->SetOffsets(FMargin(0, 0)); // Distance from top and bottom
		}

		if (ButtonFont == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("%s: No Font"), *FString(__FUNCTION__));
			return Widget;
		}

		if (AssetLoader->AssetsInChache.Num() <= 0) {
			UE_LOG(LogTemp, Warning, TEXT("%s: No assets found. Reloading assets from cache."), *FString(__FUNCTION__));
			int ReloadedAssets = AssetLoader->ReloadAssetsFromCache(); // If this list was garbage collected, reload it
			UE_LOG(LogTemp, Warning, TEXT("%s: Reloaded %s assets from cache"), *FString(__FUNCTION__), *FString::FromInt(ReloadedAssets));
		}

		// Add buttons for each item found in item cache
		for (auto& elem : AssetLoader->RefillObjectInfo)
		{
			// Add button
			UListButton* Button = NewObject<UListButton>(this, UListButton::StaticClass());
			Button->SetupButton(AssetLoader, elem.Key);
			Scrollbox->AddChild(Button);

			// Add text to button
			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));
			Button->AddChild(ButtonText);
			FString ButtonTextString;
			ButtonTextString = elem.Value.GetDisplayName();

			ButtonTextString.RemoveFromStart("SM_");
			ButtonTextString.RemoveFromEnd(FString(".uasset"));

			ButtonText->SetText(FText::FromString(ButtonTextString));		

			FSlateFontInfo FontInfo = FSlateFontInfo(ButtonFont, 18.0f);
			ButtonText->SetFont(FontInfo);	
			// *** *** *** *** *** *** 
		}
	}

	return Widget;
}


void UHUDWidget::OnUIEnabled()
{
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true; // Enables mouse cursor if widged gets enabled
}

void UHUDWidget::OnUIDisabled()
{
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false; // Disables mouse cursor if widged gets disabled
}


