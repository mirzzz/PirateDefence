// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UObject/ConstructorHelpers.h"

/**
 * 
 LoadAsset(LoadObject) : Texture, Material, SoundWave, SoundCue, ParticlesSystem, AnimMontage, BlendSpace(1D，2D，3D), AnimSequence, AnimBlueprint, SkeletalMesh
 LaodUClass(LoadClass - NewObject) : BP, Actor_BP, DataClass_BP, 
 Load, Find, LoadObject, LoadClass, NewObject, uasset, blueprint, class
   - ConstructorHelpers::FObjectFinder
 */
class UWidgetAnimation;
class UImage;
class UTextBlock;
class UCommonTextBlock;

// Plugin 통일 Namespace
namespace Nx_UtilityUI
{
	NXCODEUI_API int			GetMonitorCount();
	NXCODEUI_API FIntVector2	GetPrimaryMonitorSize();
	NXCODEUI_API FIntVector2	GetMonitorSize(int32 Pos);
	NXCODEUI_API FMonitorInfo*	GetMonitorInfo(int32 Pos);

	template <typename T>
	FORCEINLINE T* LoadWidget(UObject* Outer, UWorld* OwningObject, const FString& Path)
	{
		UClass* ClassType = LoadObject<UClass>(Outer, *Path);
		if (nullptr == ClassType)	return nullptr;

		return Cast<T>(CreateWidget<T>(OwningObject, ClassType));
	}

	template <typename T>
	FORCEINLINE void GetWidgets(UWidget* InWidget, TArray<T*>& outWidgets)
	{
		UPanelWidget* PanelWidget = Cast<UPanelWidget>(InWidget);
		if (nullptr != PanelWidget)
			GetWidgets(PanelWidget, outWidgets);
		UUserWidget* UserWidget = Cast<UUserWidget>(InWidget);
		if (nullptr != UserWidget)
			GetWidgets(UserWidget, outWidgets);
	}

	template <typename T>
	FORCEINLINE void GetWidgets(UPanelWidget* InWidget, TArray<T*>& outWidgets)
	{
		TArray<UWidget*> Widgets = InWidget->GetAllChildren();
		for (UWidget* Widget : Widgets)
		{
			if (Cast<T>(Widget))
				outWidgets.Add(Widget);

			GetWidgets(Widget, outWidgets);
		}
	}

	template <typename T>
	FORCEINLINE void GetWidgets(UUserWidget* InWidget, TArray<T*>& outWidgets)
	{
		UWidgetTree* WidgetTress = InWidget->WidgetTree;
		if (nullptr == WidgetTress)
			return;

		TArray<UWidget*> Widgets;
		WidgetTress->GetAllWidgets(Widgets);

		for (UWidget* Widget : Widgets)
		{
			if (Cast<T>(Widget))
				outWidgets.Add(Widget);

			GetWidgets(Widget, outWidgets);
		}
	}

	//Animation
	NXCODEUI_API TArray<TObjectPtr<UWidgetAnimation>> GetAnimations(UUserWidget* InWidget);
	NXCODEUI_API TObjectPtr<UWidgetAnimation> FindAnimation(UUserWidget* InWidget, const FString& InName);

	NXCODEUI_API bool IsPlayingAnimation(UUserWidget* InWidget, const FString& InName);
	NXCODEUI_API bool PlayAnimation(UUserWidget* InWidget, const FString& InName, float startTime = 0.0f, int32 loopCount = 1, EUMGSequencePlayMode::Type playType = EUMGSequencePlayMode::Forward);
	NXCODEUI_API bool StopAnimation(UUserWidget* InWidget, const FString& InName);
	NXCODEUI_API bool PauseAnimation(UUserWidget* InWidget, const FString& InName);
	NXCODEUI_API bool FinishPosAnimation(UUserWidget* InWidget, const FString& InName);

	NXCODEUI_API float GetAnimation_CurrentTime(UUserWidget* InWidget, const FString& InName);
	NXCODEUI_API float GetAnimation_EndTime(UUserWidget* InWidget, const FString& InName);

	//Widget
	NXCODEUI_API void SetColor(UImage* InImage, FLinearColor& Color);
	NXCODEUI_API void SetColor(UTextBlock* InTextBlock, FLinearColor& Color);
	NXCODEUI_API void SetColor(UCommonTextBlock* InCommonTextBlock, FLinearColor& Color);

	NXCODEUI_API void SetColor_FromSRGB(UImage* InImage, FColor& Color);
	NXCODEUI_API void SetColor_FromSRGB(UTextBlock* InTextBlock, FColor& Color);
	NXCODEUI_API void SetColor_FromSRGB(UCommonTextBlock* InCommonTextBlock, FColor& Color);

	//Slot
	NXCODEUI_API void SetPadding(UWidget* InWidget, FMargin& InMargin);
	NXCODEUI_API void SetSize(UWidget* InWidget, FVector2D& InSize);
};
