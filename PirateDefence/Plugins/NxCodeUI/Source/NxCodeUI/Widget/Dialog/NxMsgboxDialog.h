// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxPopupWindow.h"
#include "NxMsgboxDialog.generated.h"

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

DECLARE_DELEGATE_OneParam(FOnMsgBoxButtonDelegate, UNxMsgboxDialog* /* Result */);

/**
 	[MessageBox 사용법]
	UNxMsgboxDialog*	dialog = NxCodeUISubsystem::Get()->OpenMessageBox(0, EUILayer::GameMenu);
		FOnMsgBoxButtonDelegate FuncOne = FOnMsgBoxButtonDelegate::CreateLambda([&](UNxMsgboxDialog* wnd) {
			UGameplayStatics::OpenLevel(GetWorld(), "L_BaruMap");
		});
		FOnMsgBoxButtonDelegate FuncTwo = FOnMsgBoxButtonDelegate::CreateLambda([&](UNxMsgboxDialog* wnd) {
			wnd->DeactivateWidget();
		});
		dialog->Setup(FText::FromString("Title_Test"), FText::FromString("Message_Test"),
						FText::FromString("OK"), FuncOne,
						FText::FromString("Cancel"), FuncTwo);
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NXCODEUI_API UNxMsgboxDialog : public UNxPopupWindow
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

public:
	bool Open(bool bHorizontal) override;

	virtual void Setup(const FText& Title, const FText& Message,
			const FText& ButtonNameOne, FOnMsgBoxButtonDelegate FuncOne,
			const FText& ButtonNameTwo, FOnMsgBoxButtonDelegate FuncTwo);
	//virtual void Setup(const FText& Title, const FText& Message,
	//	const FText& ButtonNameOne, TFunction<UNxMsgboxDialog*> FuncOne,
	//	const FText& ButtonNameTwo, TFunction<UNxMsgboxDialog*> FuncTwo);

private:
	UFUNCTION()
	FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

private:
	FOnMsgBoxButtonDelegate OnFuncOne;
	FOnMsgBoxButtonDelegate OnFuncTwo;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Vx.MessageBox", meta = (BindWidget))
	TObjectPtr<class UCommonBorder> Border_BG;

	UPROPERTY(BlueprintReadOnly, Category = "Vx.MessageBox", meta = (BindWidget))
	TObjectPtr<class UCommonTextBlock> Text_Title;

	UPROPERTY(BlueprintReadOnly, Category = "Vx.MessageBox", meta = (BindWidget))
	TObjectPtr<class UCommonRichTextBlock> Text_Message;

	UPROPERTY(BlueprintReadOnly, Category = "Vx.MessageBox", meta = (BindWidget))
	TObjectPtr<class UNxButtonNormal> Btn_One;

	UPROPERTY(BlueprintReadOnly, Category = "Vx.MessageBox", meta = (BindWidget))
	TObjectPtr<class UNxButtonNormal> Btn_Two;
};
