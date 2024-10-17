// Fill out your copyright notice in the Description page of Project Settings.

#include "NxCodeUIUtility.h"

#include "Animation/WidgetAnimation.h"
#include "Animation/WidgetAnimationBinding.h"
#include "Animation/UMGSequencePlayer.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.h"

#include "Components/CanvasPanelSlot.h"

#include "Components/OverlaySlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBoxSlot.h"

#include "Components/WrapBoxSlot.h"
#include "Components/BackgroundBlurSlot.h"
#include "Components/BorderSlot.h"
#include "Components/ButtonSlot.h"
#include "Components/GridSlot.h"
#include "Components/SafeZoneSlot.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/StackBoxSlot.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Components/WindowTitleBarAreaSlot.h"

#include "Kismet/KismetMathLibrary.h"

namespace Nx_UtilityUI
{
	int GetMonitorCount()
	{
		FDisplayMetrics Display;
		FDisplayMetrics::RebuildDisplayMetrics(Display);
		return Display.MonitorInfo.Num();
	}

	FIntVector2 GetPrimaryMonitorSize()
	{
		FDisplayMetrics Display;
		FDisplayMetrics::RebuildDisplayMetrics(Display);

		FIntVector2 MonitorSize;
		MonitorSize.X = Display.PrimaryDisplayWidth;
		MonitorSize.Y = Display.PrimaryDisplayHeight;
		return MonitorSize;
	}

	FIntVector2 GetMonitorSize(int32 Pos)
	{
		FIntVector2 MonitorSize;

		FDisplayMetrics Display;
		FDisplayMetrics::RebuildDisplayMetrics(Display);
		if (Display.MonitorInfo.Num() > Pos)
		{
			MonitorSize.X = Display.MonitorInfo[Pos].NativeWidth;
			MonitorSize.Y = Display.MonitorInfo[Pos].NativeHeight;
		}

		return MonitorSize;
	}

	FMonitorInfo* GetMonitorInfo(int32 Pos)
	{
		FDisplayMetrics Display;
		FDisplayMetrics::RebuildDisplayMetrics(Display);
		if (Display.MonitorInfo.Num() > Pos)
			return &Display.MonitorInfo[Pos];
		return nullptr;
	}

	TArray<TObjectPtr<class UWidgetAnimation>> GetAnimations(UUserWidget* InWidget)
	{
		//const UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(InWidget->GeneratedClass);
		const UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(InWidget->GetClass());
		if (nullptr == BPClass)
			return TArray<TObjectPtr<class UWidgetAnimation>>();

		return BPClass->Animations;
	}

	TObjectPtr<UWidgetAnimation> FindAnimation(UUserWidget* InWidget, const FString& InName)
	{
		const UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(InWidget->GetClass());
		if (nullptr == BPClass)
			return nullptr;

		for (auto& ani : BPClass->Animations)
		{
			if (ani->GetName().Compare(InName))
				return ani;
		}

		return nullptr;
	}

	bool IsPlayingAnimation(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return false;
		TObjectPtr<UUMGSequencePlayer>* findSequecne = InWidget->ActiveSequencePlayers.FindByPredicate([&](const UUMGSequencePlayer* seqPlayer) { return seqPlayer->GetAnimation() == ani; });
		if (nullptr == findSequecne)
			return false;

		return (*findSequecne)->GetPlaybackStatus() == EMovieScenePlayerStatus::Playing;
	}

	bool PlayAnimation(UUserWidget* InWidget, const FString& InName, float startTime, int32 loopCount, EUMGSequencePlayMode::Type playType)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return false;

		UUMGSequencePlayer* sequence = InWidget->PlayAnimation(ani, startTime, loopCount, playType);
		if (nullptr == sequence)
			return false;

		//todo

		return true;
	}

	bool StopAnimation(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return false;

		InWidget->StopAnimation(ani);
		return true;
	}

	bool PauseAnimation(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return false;

		InWidget->PauseAnimation(ani);
		return true;
	}

	bool FinishPosAnimation(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return false;

		float EndTime = ani->GetEndTime();
		UUMGSequencePlayer* sequence = InWidget->PlayAnimation(ani, EndTime);
		if (nullptr == sequence)
			return false;

		return true;
	}

	float GetAnimation_CurrentTime(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return 0.0;
		TObjectPtr<UUMGSequencePlayer>* findSequecne = InWidget->ActiveSequencePlayers.FindByPredicate([&](const UUMGSequencePlayer* seqPlayer) { return seqPlayer->GetAnimation() == ani; });
		if (nullptr == findSequecne)
			return 0.0;

		return (*findSequecne)->GetCurrentTime().AsSeconds();
	}

	float GetAnimation_EndTime(UUserWidget* InWidget, const FString& InName)
	{
		UWidgetAnimation* ani = FindAnimation(InWidget, InName);
		if (nullptr == ani)
			return 0.0;

		return ani->GetEndTime();
	}

	void SetColor(UImage* InImage, FLinearColor& Color)
	{
		InImage->SetColorAndOpacity(Color);
	}

	void SetColor(UTextBlock* InTextBlock, FLinearColor& Color)
	{
		InTextBlock->SetColorAndOpacity(Color);
	}

	void SetColor(UCommonTextBlock* InCommonTextBlock, FLinearColor& Color)
	{
		InCommonTextBlock->SetColorAndOpacity(Color);
	}

	void SetColor_FromSRGB(UImage* InImage, FColor& Color)
	{
		FLinearColor LinearColor;
		UKismetMathLibrary::LinearColor_SetFromSRGB(LinearColor, Color);
		SetColor(InImage, LinearColor);
	}

	void SetColor_FromSRGB(UTextBlock* InTextBlock, FColor& Color)
	{
		FLinearColor LinearColor;
		UKismetMathLibrary::LinearColor_SetFromSRGB(LinearColor, Color);
		SetColor(InTextBlock, LinearColor);
	}

	void SetColor_FromSRGB(UCommonTextBlock* InCommonTextBlock, FColor& Color)
	{
		FLinearColor LinearColor;
		UKismetMathLibrary::LinearColor_SetFromSRGB(LinearColor, Color);
		SetColor(InCommonTextBlock, LinearColor);
	}

	void SetPadding(UWidget* InWidget, FMargin& InMargin)
	{
		if (nullptr == InWidget)
			return;

		auto OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(InWidget);
		if (nullptr != OverlaySlot)
		{
			OverlaySlot->SetPadding(InMargin);
			return;
		}

		auto HorizontalBoxSlot = UWidgetLayoutLibrary::SlotAsHorizontalBoxSlot(InWidget);
		if (nullptr != HorizontalBoxSlot)
		{
			HorizontalBoxSlot->SetPadding(InMargin);
			return;
		}

		auto VerticalBoxSlot = UWidgetLayoutLibrary::SlotAsHorizontalBoxSlot(InWidget);
		if (nullptr != VerticalBoxSlot)
		{
			VerticalBoxSlot->SetPadding(InMargin);
			return;
		}

		auto SizeBoxSlot = UWidgetLayoutLibrary::SlotAsSizeBoxSlot(InWidget);
		if (nullptr != SizeBoxSlot)
		{
			SizeBoxSlot->SetPadding(InMargin);
			return;
		}

		auto WrapBoxSlot = UWidgetLayoutLibrary::SlotAsWrapBoxSlot(InWidget);
		if (nullptr != WrapBoxSlot)
		{
			WrapBoxSlot->SetPadding(InMargin);
			return;
		}

		auto BorderSlot = UWidgetLayoutLibrary::SlotAsBorderSlot(InWidget);
		if (nullptr != BorderSlot)
		{
			SizeBoxSlot->SetPadding(InMargin);
			return;
		}

		auto GridSlot = UWidgetLayoutLibrary::SlotAsGridSlot(InWidget);
		if (nullptr != GridSlot)
		{
			GridSlot->SetPadding(InMargin);
			return;
		}

		auto ScrollBoxSlot = UWidgetLayoutLibrary::SlotAsScrollBoxSlot(InWidget);
		if (nullptr != ScrollBoxSlot)
		{
			ScrollBoxSlot->SetPadding(InMargin);
			return;
		}

		auto WidgetSwitcherSlot = UWidgetLayoutLibrary::SlotAsWidgetSwitcherSlot(InWidget);
		if (nullptr != WidgetSwitcherSlot)
		{
			WidgetSwitcherSlot->SetPadding(InMargin);
			return;
		}
	}

	void SetSize(UWidget* InWidget, FVector2D& InSize)
	{
		if (nullptr == InWidget)
			return;

		auto CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(InWidget);
		if (nullptr != CanvasSlot)
		{
			CanvasSlot->SetSize(InSize);
			return;
		}
	}
};