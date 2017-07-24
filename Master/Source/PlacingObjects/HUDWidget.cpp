// Fill out your copyright notice in the Description page of Project Settings.

#include "PlacingObjects.h"
#include "HUDWidget.h"

UHUDWidget::UHUDWidget(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Engine/EngineFonts/DroidSansMono"));
	ButtonFont = FontObject.Object;
}

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// UScrollBox *panel = NewObject<UScrollBox>(this, UScrollBox::StaticClass());

	//UScrollBox *panel =  WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());



	//UTextBlock* tstText = NewObject<UTextBlock>(this,UTextBlock::StaticClass());
	//tstText->SetText(FText::FromString(TEXT("Test Button")));
	//UButton *tstBtn = NewObject<UButton>(this, UButton::StaticClass());
	//tstBtn->AddChild(tstText);
	//panel->AddChild(tstBtn);

	//panel->SetVisibility(ESlateVisibility::Visible);

	//UE_LOG(LogTemp, Warning, TEXT("Construction"));

	// Bind delegates here.
}

TSharedRef<SWidget> UHUDWidget::RebuildWidget()
{
	// TODO for Database
	//if (AssetLoader == nullptr)
	//{
	//	for (TObjectIterator<ARAssetLoader> Itr; Itr; ++Itr)
	//	{
	//		AssetLoader = *Itr;
	//	}
	//}

	//check(AssetLoader);

	if (AssetLoader == nullptr)
	{
		for (TObjectIterator<ACacheAssetLoader> Itr; Itr; ++Itr)
		{
			AssetLoader = *Itr;
		}
	}

	check(AssetLoader);

	// https://answers.unrealengine.com/storage/attachments/103428-capture.png
	UPanelWidget* RootWidget = Cast<UPanelWidget>(GetRootWidget());

	if (RootWidget == nullptr)
	{
		RootWidget = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootWidget"));

		UCanvasPanelSlot* RootWidgetSlot = Cast<UCanvasPanelSlot>(RootWidget->Slot);

		if (RootWidgetSlot)
		{
			RootWidgetSlot->SetAnchors(FAnchors(0, 0, 1, 1));
			RootWidgetSlot->SetOffsets(FMargin(0, 0));
		}

		WidgetTree->RootWidget = RootWidget;
	}

	TSharedRef<SWidget> Widget = Super::RebuildWidget();

	if (RootWidget && WidgetTree)
	{
		UScrollBox* Scrollbox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("Scroll list"));
		RootWidget->AddChild(Scrollbox);
		UCanvasPanelSlot* ScrollboxSlot = Cast<UCanvasPanelSlot>(Scrollbox->Slot);

		if (ScrollboxSlot)
		{
			// ScrollboxSlot->SetAnchors(FAnchors(0.9f, 0, 1, 1)); // 0.9 means it always takes 10% of the screen
			ScrollboxSlot->SetAnchors(FAnchors(0.75f, 0, 1, 1)); // 0.9 means it always takes 10% of the screen

			ScrollboxSlot->SetOffsets(FMargin(0, 0)); // Distance from top and bottom
		}

		//TMap<FString, FString> TestMap;
		//TestMap.Add(FString("ID_8e4n48234nh59345n9435h98z345"), FString("Blah"));
		//TestMap.Add(FString("ID_ksjdbf934tziub983z4tr4389z53"), FString("Blubb"));
		//TestMap.Add(FString("ID_23ihg943rh4305h90zu45rn439th"), FString("Test"));
		//TestMap.Add(FString("ID_woreih4308u32495h043h6039032"), FString("Nothing"));

		// Get all asset names 
		//for (auto elem : TestMap)
		//	//for (auto& elem : AssetLoader->IDToGUIParts)
		//{

		//	// UE_LOG(LogTemp, Warning, TEXT("Asset-ID %s Name: %s Image: %s"), *elem.Key, *elem.Value.Key, *elem.Value.Value);
		//	UListButton* Button = NewObject<UListButton>(this, UListButton::StaticClass());
		//	Button->SetupButton(AssetLoader, elem.Key);

		//	Scrollbox->AddChild(Button);

		//	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));
		//	Button->AddChild(ButtonText);


		//	ButtonText->SetText(FText::FromString(elem.Value));
		//	// TODO use for MongoDB ButtonText->SetText(FText::FromString(elem.Value.Key));		
		//}


		if (ButtonFont == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("No Font"));
			return Widget;
		}

		for (auto& elem : AssetLoader->AssetsInChache)
		{
			UListButton* Button = NewObject<UListButton>(this, UListButton::StaticClass());
			Button->SetupButton(AssetLoader, elem);

			Scrollbox->AddChild(Button);

			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));
			Button->AddChild(ButtonText);

			FString ButtonTextString = elem;
	
			ButtonTextString.RemoveFromStart("SM_");
			ButtonTextString.RemoveFromEnd(FString(".uasset"));
			ButtonText->SetText(FText::FromString(ButtonTextString));

			FSlateFontInfo FontInfo = FSlateFontInfo(ButtonFont, 18.0f);
			ButtonText->SetFont(FontInfo);


		}




		//for (size_t i = 0; i < 10; i++)
		//{
		//	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button"));
		//	Scrollbox->AddChild(Button);

		//	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));
		//	Button->AddChild(ButtonText);
		//	ButtonText->SetText(FText::FromString(FString("Click me!")));
		//}



		//UTextBlock* Textbox = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TEXTBOX"));
		//RootWidget->AddChild(Textbox);

		//Textbox->SetText(FText::FromString(FString("Hallo")));
		//UCanvasPanelSlot* TextboxSlot = Cast<UCanvasPanelSlot>(Textbox->Slot);

		//if (TextboxSlot)
		//{
		//	TextboxSlot->SetAnchors(FAnchors(0, 0, 1, 1));
		//	TextboxSlot->SetOffsets(FMargin(500, 200));
		//	TextboxSlot->SetAutoSize(true);
		//}
	}

	return Widget;
}

//
//FReply UHUDWidget::OnButtonClicked(FString f)
//{
//	return FReply::Handled();
//}

void UHUDWidget::OnUIEnabled()
{
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
}

void UHUDWidget::OnUIDisabled()
{
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = false;
}


