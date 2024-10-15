// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Internationalization/Text.h"
#include "NxLoadingUserWidget.generated.h"

UENUM(BlueprintType)
enum class E_WidgetLoadingLayout : uint8
{
	// 일반 레이아웃
	DefaultLayout,
	// 로딩바 레이아웃
	ProgressLayout,
};

// Custom Loading Status 정보
USTRUCT()
struct NXLOADINGUSERWIDGET_API FNxLoadingWidgetLayoutInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName MapName;

	UPROPERTY()
	E_WidgetLoadingLayout  Layout = E_WidgetLoadingLayout::DefaultLayout;

	UPROPERTY()
	TObjectPtr<class UTexture2D>	BackImage;

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString Desc;
};

/**
 * LoadingWidget
 *	- SimpleLayout		화면 연출
 *	- ProgressbarLayout 화면 연출
 */
UCLASS()
class NXLOADINGUSERWIDGET_API UNxLoadingUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	void SetWidgetLayout(FNxLoadingWidgetLayoutInfo& layout);

	UFUNCTION()
	FText GetLoadingProgressText();

protected:

	FNxLoadingWidgetLayoutInfo LayoutInfo;

public:

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UWidgetSwitcher>	SW_Layout;

	// Simple Layout
	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonBorder>		BD_SimpleBackground;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UImage>			IM_SimpleBackground;
		
	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonTextBlock>	TB_SimpleTitle;

	// Progress Layout
	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonBorder>		BD_ProgressBackground;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UImage>			IM_ProgressBackground;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonTextBlock>	TB_ProgressTitle;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonTextBlock>	TB_ProgressDesc;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UProgressBar>		PB_ProgressLoading;

	UPROPERTY() //Meta = (BindWidget)
	TObjectPtr<class UCommonTextBlock>	PB_ProgressText;

};
