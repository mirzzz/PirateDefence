// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Frame/NxUIStruct.h"
#include "NxCodeUISettings.generated.h"

class UCommonBorderStyle;
class UCommonButtonStyle;
class UCommonTextStyle;

USTRUCT()
struct NXCODEUI_API FNxStylePack
{
	GENERATED_USTRUCT_BODY()

	TSubclassOf<UCommonBorderStyle>	GetBorder(TSubclassOf<UCommonBorderStyle> Style);
	TSubclassOf<UCommonButtonStyle>	GetButton(TSubclassOf<UCommonButtonStyle> Style);
	TSubclassOf<UCommonTextStyle>	GetText(TSubclassOf<UCommonTextStyle> Style);

public:
	UPROPERTY(EditAnywhere, Category = CodeUI)
	TArray<TSubclassOf<UCommonBorderStyle>>	Borders;

	UPROPERTY(EditAnywhere, Category = CodeUI)
	TArray<TSubclassOf<UCommonButtonStyle>>	Buttons;

	UPROPERTY(EditAnywhere, Category = CodeUI)
	TArray<TSubclassOf<UCommonTextStyle>>	Texts;
};
 
/**
 * 
 */
UCLASS(Config = Plugin, DefaultConfig, meta = (DisplayName = "UISettings"))
class NXCODEUI_API UNxCodeUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ~UDeveloperSettings
	UNxCodeUISettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 프로젝트 Category 설정 
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Nx"); }
	virtual FName GetSectionName() const override { return FName("NxCodeUI"); }

#if WITH_EDITOR
		// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	// 활성화 여부
	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	bool  Enable = false;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TArray<FNxFrameUnit> FrameInfos;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	FString BPPath;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxFrameWidget> SubclassMainFrame;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxFrameWidget> SubclassCommonFrame;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxMsgboxDialog> MessageBoxDlg;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxMsgboxExtendDialog> MessageBoxOneDlg;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxMsgboxExtendDialog> MessageBoxTwoDlg;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxMsgboxExtendDialog> MessageBoxThreeDlg;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxNoticesDialog> NoticesDlg;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxProgressIndicatorDialog> ProgressIndicatorDlg;
		
	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	FString StyleName;

	UPROPERTY(config, EditAnywhere, Category = CodeUI)
	TMap<FString, FNxStylePack> StylePacks;
};
