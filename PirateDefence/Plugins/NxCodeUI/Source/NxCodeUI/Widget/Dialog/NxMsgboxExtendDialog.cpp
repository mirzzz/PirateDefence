// Fill out your copyright notice in the Description page of Project Settings.

#include "NxMsgboxExtendDialog.h"

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


void UNxMsgboxExtendDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	//if(nullptr != Border_PopupBackground)
	//	Border_PopupBackground->OnMouseButtonDownEvent.BindDynamic(this, &UNxMsgboxExtendDialog::HandleTapToCloseZoneMouseButtonDown);

	if (nullptr != Button_0)
		ButtonNormals.Add(Button_0);
	if (nullptr != Button_1)
		ButtonNormals.Add(Button_1);
	if (nullptr != Button_2)
		ButtonNormals.Add(Button_2);
}

void UNxMsgboxExtendDialog::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UNxMsgboxExtendDialog::Open(bool bHorizontal)
{
	if (false == Super::Open(bHorizontal))
		return false;

	return true;
}

void UNxMsgboxExtendDialog::Setup(FNxMsgboxButtonInfo& Info)
{
	MessageBoxInfo = Info;

	if(nullptr != Text_Title)
		Text_Title->SetText(Info.Title);
	if(nullptr != Text_Message)
		Text_Message->SetText(Info.Message);

	for (int i=0; i< Info.Buttons.Num(); ++i)
	{
		if (nullptr != ButtonNormals[i])
		{
			auto func = Info.Buttons[i].Func;
			ButtonNormals[i]->SetButtonText(Info.Buttons[i].Name);
			ButtonNormals[i]->OnClicked().AddLambda([this, func]() {
				func.Execute(this);
			});
		}
	}
}

// 마우스 클릭 이벤트 발생시 대화창 닫기
FEventReply UNxMsgboxExtendDialog::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
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
void UNxMsgboxExtendDialog::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	//CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} 설정되지 않은 속성: CancelAction.")), FText::FromString(GetName())));
}
#endif