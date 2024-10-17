// Fill out your copyright notice in the Description page of Project Settings.


#include "NxActivatableWidgetComponent.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Animation/UMGSequencePlayer.h"
#include "MovieScene.h"

#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"

#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

//Text
#include "Localization/NxLocalizationSubsystem.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif 

UNxActivatableWidgetComponent::UNxActivatableWidgetComponent(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
}

bool UNxActivatableWidgetComponent::Initialize()
{
	if (false == Super::Initialize())
		return false;

	return true;
}

void UNxActivatableWidgetComponent::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InitCollectionChild();
	InitLocalization();
}

void UNxActivatableWidgetComponent::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNxActivatableWidgetComponent::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNxActivatableWidgetComponent::NativeOnActivated()
{
	//LogUI(Log);

	Super::NativeOnActivated();
}

void UNxActivatableWidgetComponent::NativeOnDeactivated()
{
	//LogUI(Log);

	Super::NativeOnDeactivated();
}

bool UNxActivatableWidgetComponent::ProcessInputKey(FInputKeyEventArgs& EventArgs)
{
	return Super::ProcessInputKey(EventArgs);
}

FEventReply UNxActivatableWidgetComponent::ProcessMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

FEventReply UNxActivatableWidgetComponent::ProcessMouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

FEventReply UNxActivatableWidgetComponent::ProcessMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

FEventReply UNxActivatableWidgetComponent::ProcessMouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

TOptional<FUIInputConfig> UNxActivatableWidgetComponent::GetDesiredInputConfig() const
{
	return Super::GetDesiredInputConfig();
}

bool UNxActivatableWidgetComponent::Open(bool bHorizontal)
{
	if (false == Super::Open(bHorizontal))
		return false;

	return true;
}

void UNxActivatableWidgetComponent::Close()
{
	Super::Close();
}

void UNxActivatableWidgetComponent::UpdateControl()
{
	Super::UpdateControl();
}

void UNxActivatableWidgetComponent::InitCollectionChild()
{
	Super::InitCollectionChild();

	UpdateStyle();
}

void UNxActivatableWidgetComponent::InitLocalization()
{
	Super::InitLocalization();
}

void UNxActivatableWidgetComponent::UpdateStyle()
{
	for (auto itr : IncludeWidgets)
	{
		UCommonBorder* border = Cast<UCommonBorder>(itr.Value);
		if (nullptr != border)
		{
			auto s = border->Style;
		}

		UCommonButtonBase* button = Cast<UCommonButtonBase>(itr.Value);
		if (nullptr != button)
		{

		}

		UCommonTextBlock* text = Cast<UCommonTextBlock>(itr.Value);
		if (nullptr != text)
		{

		}
	}
}

#define LOCTEXT_NAMESPACE "VxLoc"

#if WITH_EDITOR

void UNxActivatableWidgetComponent::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// 함수 구현 체크
	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UNxActivatableWidgetComponent, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UNxActivatableWidgetComponent::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget() 이 구현되지 않았으므로, 화면에서 게임 패드를 사용하는 데 문제가 있습니다."));
		}
		else
		{
			// TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget() 이 구현되지 않았으므로, 화면에서 게임 패드를 사용하는 데 문제가 있습니다. 네이티브 기본 클래스에서 구현된 경우 이 메시지를 무시해도 됩니다."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE
