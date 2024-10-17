// Fill out your copyright notice in the Description page of Project Settings.

#include "NxNoticesDialog.h"

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


void UNxNoticesDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxNoticesDialog::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	switch (EnumCheckClose)
	{
	case Time:
		OpenAddedTime += InDeltaTime;
		if (OpenAddedTime >= DurationTime)
		{
			Close();
		}
		break;
	case Event:
		if (FuncCloseEvent.Execute())
		{
			Close();
		}
		break;
	default:
		break;
	}
}

bool UNxNoticesDialog::Open(bool bHorizontal)
{
	if (false == Super::Open(bHorizontal))
		return false;
	
	OpenAddedTime = 0;

	return true;
}

void UNxNoticesDialog::Setup(const FText& Message, float Duration)
{
	EnumCheckClose = E_NoticesCloseEvent::Time;

	if (nullptr != Text_Message)
		Text_Message->SetText(Message);

	DurationTime = Duration >= 1 ? Duration : 1;
}

void UNxNoticesDialog::Setup(const FText& Message, FOnNoticesDialogCloseDelegate Event)
{
	EnumCheckClose = E_NoticesCloseEvent::Event;

	if (nullptr != Text_Message)
		Text_Message->SetText(Message);

	FuncCloseEvent = Event;
}

#if WITH_EDITOR
void UNxNoticesDialog::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	//CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} 설정되지 않은 속성: CancelAction.")), FText::FromString(GetName())));
}
#endif