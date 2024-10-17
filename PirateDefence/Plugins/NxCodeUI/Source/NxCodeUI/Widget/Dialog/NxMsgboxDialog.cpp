// Fill out your copyright notice in the Description page of Project Settings.

#include "NxMsgboxDialog.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#include "CommonBorder.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"
#include "Input/Reply.h"
#include "NxButtonNormal.h"


void UNxMsgboxDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if(nullptr != Border_BG)
		Border_BG->OnMouseButtonDownEvent.BindDynamic(this, &UNxMsgboxDialog::HandleTapToCloseZoneMouseButtonDown);
}

bool UNxMsgboxDialog::Open(bool bHorizontal)
{
	if (false == Super::Open(bHorizontal))
		return false;

	return true;
}

void UNxMsgboxDialog::Setup(const FText& Title, const FText& Message,
	const FText& ButtonNameOne, FOnMsgBoxButtonDelegate FuncOne,
	const FText& ButtonNameTwo, FOnMsgBoxButtonDelegate FuncTwo)
{
	Text_Title->SetText(Title);
	Text_Message->SetText(Message);
	Btn_One->SetButtonText(ButtonNameOne);
	Btn_Two->SetButtonText(ButtonNameTwo);

	OnFuncOne = FuncOne;
	OnFuncTwo = FuncTwo;

	Btn_One->OnClicked().AddLambda([this]() {
		OnFuncOne.Execute(this);
	});

	Btn_Two->OnClicked().AddLambda([this]() {
		OnFuncTwo.Execute(this);
	});
}

// 마우스 클릭 이벤트 발생시 대화창 닫기
FEventReply UNxMsgboxDialog::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	// FEventReply: 사용자가 이벤트를 처리하고 기본 UI계층에 정보를 반환할 정보를 담은 구조체이다.
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨

	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//CloseMessageWindow(EVxUI_DialogResult::Declined);

		Reply.NativeReply = FReply::Handled();	// 이벤트 처리 됨
	}

	return Reply;
}


#if WITH_EDITOR
void UNxMsgboxDialog::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	//CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} 설정되지 않은 속성: CancelAction.")), FText::FromString(GetName())));
}
#endif