// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxPopupWindow.h"
#include "NxNoticesDialog.generated.h"

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

enum E_NoticesCloseEvent
{
	Time,
	Event
};

DECLARE_DELEGATE_RetVal(bool, FOnNoticesDialogCloseDelegate);

UCLASS(Abstract, BlueprintType, Blueprintable)
class NXCODEUI_API UNxNoticesDialog : public UNxPopupWindow
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

public:
	bool Open(bool bHorizontal) override;

	virtual void Setup(const FText& Message, float Duration);
	virtual void Setup(const FText& Message, FOnNoticesDialogCloseDelegate Event);

protected:
	float DurationTime;
	FOnNoticesDialogCloseDelegate FuncCloseEvent;
	E_NoticesCloseEvent	EnumCheckClose;

	float OpenAddedTime;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Nx.Notices", meta = (BindWidget))
	TObjectPtr<class UCommonBorder> Border_PopupBackground;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.Notices", meta = (BindWidget))
	TObjectPtr<class UCommonTextBlock> Text_Message;
};
