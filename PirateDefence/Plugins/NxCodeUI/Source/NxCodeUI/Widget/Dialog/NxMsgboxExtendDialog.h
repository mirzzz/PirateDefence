// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxPopupWindow.h"
#include "NxMsgboxExtendDialog.generated.h"

class UBackgroundBlur;
class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

DECLARE_DELEGATE_OneParam(FOnMsgboxExtendButtonDelegate, UNxMsgboxExtendDialog* /* Result */);

USTRUCT(BlueprintType)
struct NXCODEUI_API FNxMsgboxButtonEvent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FText Name;

	FOnMsgboxExtendButtonDelegate Func;
};

USTRUCT(BlueprintType)
struct NXCODEUI_API FNxMsgboxButtonInfo
{
	GENERATED_BODY()

public:
	void AddButton(FText InName, FOnMsgboxExtendButtonDelegate InFunc) {
		FNxMsgboxButtonEvent buttonInfo;
		buttonInfo.Name = InName;
		buttonInfo.Func = InFunc;
		Buttons.Add(buttonInfo);
	}
	bool CheckButtonCount(int Count)
	{
		return Buttons.Num() == Count ? true : false;
	}

public:
	UPROPERTY()
	FText Title;
	UPROPERTY()
	FText Message;
	UPROPERTY()
	TArray<FNxMsgboxButtonEvent> Buttons;
};

/**
 * 
	FNxMsgboxButtonInfo Info;
	Info.Message = FText::FromString("Message");
	FVxUIMessageBoxButtonDelegate2 FuncOne = FVxUIMessageBoxButtonDelegate2::CreateLambda([&](UNxMsgboxExtendDialog* wnd) {
		wnd->Close();
		});
	FVxUIMessageBoxButtonDelegate2 FuncTwo = FVxUIMessageBoxButtonDelegate2::CreateLambda([&](UNxMsgboxExtendDialog* wnd) {
		wnd->Close();
		});
	FVxUIMessageBoxButtonDelegate2 FuncThree = FVxUIMessageBoxButtonDelegate2::CreateLambda([&](UNxMsgboxExtendDialog* wnd) {
		wnd->Close();
		});
	Info.AddButton(FText::FromString("OK"), FuncOne);
	Info.AddButton(FText::FromString("Cancel"), FuncTwo);
	Info.AddButton(FText::FromString("Close"), FuncThree);
	NxCodeUISubsystem::Get()->OpenMessageBoxThree(0, EUILayer::Dialog, Info);
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NXCODEUI_API UNxMsgboxExtendDialog : public UNxPopupWindow
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

public:
	bool Open(bool bHorizontal) override;

	virtual void Setup(FNxMsgboxButtonInfo& Info);

private:
	UFUNCTION()
	FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

private:
	FNxMsgboxButtonInfo MessageBoxInfo;

	TArray<TObjectPtr<class UNxButtonNormal>> ButtonNormals;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UBackgroundBlur> BackgroundBlur;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UCommonBorder> Border_PopupBackground;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Text_Title;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Text_Message;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UNxButtonNormal> Button_0;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UNxButtonNormal> Button_1;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.MsgBoxExtend", meta = (BindWidgetOptional))
	TObjectPtr<class UNxButtonNormal> Button_2;
};
