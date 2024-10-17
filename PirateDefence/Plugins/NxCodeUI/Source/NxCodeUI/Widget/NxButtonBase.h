// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Frame/NxUIStruct.h"
#include "NxButtonBase.generated.h"

USTRUCT()
struct FNxButtonOverrideIcon
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Nx|Button")
	FMargin Padding;

	UPROPERTY(EditAnywhere, Category = "Nx|Button")
	FSlateBrush	Brush;
};

USTRUCT()
struct FNxButtonOverrideText
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Nx|Button")
	FMargin Padding;

	UPROPERTY(EditAnywhere, Category = "Nx|Button")
	FText Text;
};

/**
 * 
 */
UCLASS(Abstract)
class NXCODEUI_API UNxButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
protected:
	// UUserWidget interface
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;

	virtual void NativeOnClicked() override;

	virtual void UpdateInputActionWidget() override;
	// End of UCommonButtonBase interface

protected:
	virtual void OnClickedEvent();
	virtual void OnDoubleClickedEvent();
	virtual void OnPressedEvent();
	virtual void OnReleasedEvent();
	virtual void OnHoveredEvent();
	virtual void OnUnhoveredEvent();
	virtual void OnFocusReceivedEvent();
	virtual void OnFocusLostEvent();

	virtual void CollectionChild();

	virtual void RefreshSizeMargin();
	virtual void RefreshButtonIcon() {}
	virtual void RefreshButtonText() {}

	FNxButtonAniInfo* GetAniInfo(E_ButtonEvent ButtonEvent);

public:
	UFUNCTION(BlueprintCallable)
	void SetButtonIcon(const FSlateBrush& InBrush, int InPos = 0);

	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText, int InPos = 0);

	// Animation 처리
	bool  IsExistAnimation(const FName& animationName);
	bool  IsPlayingAnimation(const FName& animationName);

	UWidgetAnimation* PlayAnimation(const FName& animationName, float startTime = 0.0f, int32 loopCount = 1, EUMGSequencePlayMode::Type playType = EUMGSequencePlayMode::Forward);
	UWidgetAnimation* StopAnimation(const FName& animationName);
	UWidgetAnimation* PauseAnimation(const FName& animationName);
	float GetAnimation_CurrentTime(const FName& animationName);
	float GetAnimation_EndTime(const FName& animationName);

	void  GetAnimationList(OUT TArray<FName>& animationList);

	FWidgetAnimationDynamicEvent OnStartEvent;
	FWidgetAnimationDynamicEvent OnEndEvent;

	UFUNCTION()
	virtual void WidgetAnimationStarted();
	UFUNCTION()
	virtual void WidgetAnimationFinished();

	UWidgetAnimation* CurrentWidgetAnimation;

protected:
	UWidgetBlueprintGeneratedClass* BPClass;				// Blueprint Class
	TMap<FName, UWidget*>			IncludeWidgets;			// 포함된 Widget 수집
	TMap<FName, UWidgetAnimation*>	IncludeAnimations;		// 포함된 Anim 수집

protected:
	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_Size : 1;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (editcondition = "bOverride_Size"))
	FVector2f ButtonSize;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_ButtonPadding : 1;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (editcondition = "bOverride_ButtonPadding"))
	FMargin ButtonPadding;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_RootBoxPadding : 1;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (editcondition = "bOverride_RootBoxPadding"))
	FMargin RootBoxPadding;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_ButtonIcon : 1;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (editcondition = "bOverride_ButtonIcon"))
	TArray<FNxButtonOverrideIcon> ButtonIcons;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;

	UPROPERTY(EditAnywhere, Category = "Nx|Button", meta = (editcondition = "bOverride_ButtonText"))
	TArray<FNxButtonOverrideText> ButtonTexts;

	UPROPERTY(EditAnywhere, Category = "Nx|Button")
	TArray<FNxButtonAniInfo> AniInfos;

	UPROPERTY(BlueprintReadOnly, Category = "UI.Button", meta = (BindWidgetOptional))
	TObjectPtr<class USizeBox> RootSizeBox;

	UPROPERTY(BlueprintReadOnly, Category = "UI.Button", meta = (BindWidgetOptional))
	TObjectPtr<class UHorizontalBox> RootHorizontalBox;

	UPROPERTY(BlueprintReadOnly, Category = "UI.Button", meta = (BindWidgetOptional))
	TObjectPtr<class UVerticalBox> RootVerticalBox;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nx|Button|Sound")
	TObjectPtr<USoundBase> SoundIconHover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nx|Button|Sound")
	TObjectPtr<USoundBase> SoundIconClick;
};
