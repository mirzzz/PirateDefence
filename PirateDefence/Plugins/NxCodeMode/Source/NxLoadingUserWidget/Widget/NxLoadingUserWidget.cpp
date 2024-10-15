// Fill out your copyright notice in the Description page of Project Settings.


#include "NxLoadingUserWidget.h"
#include "Engine/Texture2D.h"
#include "CommonTextBlock.h"
#include "CommonBorder.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetSwitcher.h"
#include "NxBaseLog.h"


void UNxLoadingUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Widget Bind....
	SW_Layout				= Cast<UWidgetSwitcher>(GetWidgetFromName(TEXT("Bind_LoadingLayoutSwitch")));

	NxCheckReturn(SW_Layout);

	// Simple (Switcher:0)
	BD_SimpleBackground		= Cast<UCommonBorder>(GetWidgetFromName(TEXT("Bind_SimpleBorderBack")));
	IM_SimpleBackground		= Cast<UImage>(GetWidgetFromName(TEXT("Bind_SimpleImageBack")));
	TB_SimpleTitle			= Cast<UCommonTextBlock>(GetWidgetFromName(TEXT("Bind_SimpleTitleText")));
	
	// Progress (Switcher:1)
	BD_ProgressBackground	= Cast<UCommonBorder>(GetWidgetFromName(TEXT("Bind_ProgressBorderBack")));
	IM_ProgressBackground	= Cast<UImage>(GetWidgetFromName(TEXT("Bind_ProgressImageBack")));
	TB_ProgressTitle		= Cast<UCommonTextBlock>(GetWidgetFromName(TEXT("Bind_ProgressTitleText")));
	TB_ProgressDesc			= Cast<UCommonTextBlock>(GetWidgetFromName(TEXT("Bind_ProgressDescText")));
	PB_ProgressLoading		= Cast<UProgressBar>(GetWidgetFromName(TEXT("Bind_ProgressBar")));
	PB_ProgressText			= Cast<UCommonTextBlock>(GetWidgetFromName(TEXT("Bind_ProgressBarText")));
}

void UNxLoadingUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNxLoadingUserWidget::SetWidgetLayout(FNxLoadingWidgetLayoutInfo& layout)
{
	LayoutInfo = layout;

	if (SW_Layout != nullptr)
	{
		switch (LayoutInfo.Layout)
		{
		case E_WidgetLoadingLayout::DefaultLayout:
		{
			SW_Layout->SetActiveWidgetIndex(0);		// 기본 로딩 화면

			if (IM_SimpleBackground)
			{
				IM_SimpleBackground->SetBrushFromTexture(LayoutInfo.BackImage);

			}
			TB_SimpleTitle->SetText(FText::FromString(LayoutInfo.Title));
		}
		break;

		case E_WidgetLoadingLayout::ProgressLayout:
		{
			SW_Layout->SetActiveWidgetIndex(1);		// 프로그레스바 로딩 화면

			if (IM_ProgressBackground)
			{
				IM_ProgressBackground->SetBrushFromTexture(LayoutInfo.BackImage);
			}

			if (TB_ProgressTitle) TB_ProgressTitle->SetText(FText::FromString(LayoutInfo.Title));
			if (TB_ProgressDesc)  TB_ProgressDesc->SetText(FText::FromString(LayoutInfo.Desc));

			if (PB_ProgressText)
			{
				// Text Binding...
				PB_ProgressText->TextDelegate.BindDynamic(this, &UNxLoadingUserWidget::GetLoadingProgressText);
			}
		}
		break;
		}
	}
}

FText UNxLoadingUserWidget::GetLoadingProgressText()
{
	if(LayoutInfo.Layout != E_WidgetLoadingLayout::ProgressLayout)
		return FText();

	float LoadPercentage = GetAsyncLoadPercentage(LayoutInfo.MapName);

	if (LoadPercentage < 0)
	{
		LoadPercentage = 100.f;
	}

	if (PB_ProgressLoading)
	{
		PB_ProgressLoading->SetPercent(LoadPercentage);
	}

	return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::TruncToInt(LoadPercentage)));
}

