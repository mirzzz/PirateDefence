// Fill out your copyright notice in the Description page of Project Settings.


#include "NxActivatableWidgetWindow.h"
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

#include "NxActivatableWidgetComponent.h"

//Text
#include "Localization/NxLocalizationSubsystem.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.h"

#include "NxBaseLog.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif 

UNxActivatableWidgetWindow::UNxActivatableWidgetWindow(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	bRootBorderClickClose = true;
}

bool UNxActivatableWidgetWindow::Initialize()
{
	if (false == Super::Initialize())
		return false;

	return true;
}

void UNxActivatableWidgetWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (nullptr != RootBorder)
	{
		RootBorder->OnMouseButtonDownEvent.BindDynamic(this, &UNxActivatableWidgetWindow::ProcessMouseButtonDown);
		RootBorder->OnMouseButtonUpEvent.BindDynamic(this, &UNxActivatableWidgetWindow::ProcessMouseButtonUp);
		RootBorder->OnMouseMoveEvent.BindDynamic(this, &UNxActivatableWidgetWindow::ProcessMouseMove);
		RootBorder->OnMouseDoubleClickEvent.BindDynamic(this, &UNxActivatableWidgetWindow::ProcessMouseDoubleClick);
	}

	InitCollectionChild();
	InitLocalization();
}

void UNxActivatableWidgetWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력키 핸들 등록
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		// 새로운 입력처리 활성화
		BeforeInputKeyHandler = ViewportClient->OnOverrideInputKey();
		ViewportClient->OnOverrideInputKey().BindUObject(this, &UNxActivatableWidgetWindow::ProcessInputKey);
	}
}

void UNxActivatableWidgetWindow::NativeDestruct()
{
	// 입력키 핸들 등록
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		// 되돌림.
		ViewportClient->OnOverrideInputKey() = BeforeInputKeyHandler;
		BeforeInputKeyHandler.Unbind();
	}

	//if (nullptr != RootBorder)
	//{
	//	RootBorder->OnMouseButtonDownEvent.Unbind();
	//	RootBorder->OnMouseButtonUpEvent.Unbind();
	//	RootBorder->OnMouseMoveEvent.Unbind();
	//	RootBorder->OnMouseDoubleClickEvent.Unbind();
	//}

	Super::NativeDestruct();
}

void UNxActivatableWidgetWindow::NativeOnActivated()
{
	//LogUI(Log);

	Super::NativeOnActivated();
}

void UNxActivatableWidgetWindow::NativeOnDeactivated()
{
	//LogUI(Log);

	Super::NativeOnDeactivated();
}

bool UNxActivatableWidgetWindow::ProcessInputKey(FInputKeyEventArgs& EventArgs)
{
	return Super::ProcessInputKey(EventArgs);
}

FEventReply UNxActivatableWidgetWindow::ProcessMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨

	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (true == bRootBorderClickClose)
		{
			Close();

			Reply.NativeReply = FReply::Handled();
		}
	}

	return Reply;
}

FEventReply UNxActivatableWidgetWindow::ProcessMouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

FEventReply UNxActivatableWidgetWindow::ProcessMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

FEventReply UNxActivatableWidgetWindow::ProcessMouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();	// 이벤트 처리 안됨
	return Reply;
}

TOptional<FUIInputConfig> UNxActivatableWidgetWindow::GetDesiredInputConfig() const
{
	return Super::GetDesiredInputConfig();
}

bool UNxActivatableWidgetWindow::Open(bool bHorizontal)
{
	if (false == Super::Open(bHorizontal))
		return false;

	auto components = GetComponents();
	for (auto component : components)
		component->Open(bHorizontal);

	return true;
}

void UNxActivatableWidgetWindow::Close()
{
	auto components = GetComponents();
	for (auto component : components)
		component->Close ();

	Super::Close();
}

void UNxActivatableWidgetWindow::UpdateControl()
{
	Super::UpdateControl();

	//auto components = GetComponents();
	//for (auto component : components)
	//	component->UpdateControl();
}

TArray<UNxActivatableWidgetComponent*> UNxActivatableWidgetWindow::GetComponents()
{
	TArray<UNxActivatableWidgetComponent*> Components;

	TArray<UWidget*> childWidgets;
	if (WidgetTree != nullptr)
	{
		// UserWidget.WidgetTree
		WidgetTree->GetAllWidgets(childWidgets);
		for (auto& comp : childWidgets)
		{
			UWidget* widget = comp;
			if (widget == nullptr || widget->IsValidLowLevel() == false)
				continue;
			UNxActivatableWidgetComponent* uiBaseComponent = Cast<UNxActivatableWidgetComponent>(comp);
			if (uiBaseComponent == nullptr)
				continue;

			Components.Add(uiBaseComponent);
		}
	}

	return Components;
}


void UNxActivatableWidgetWindow::InitCollectionChild()
{
	Super::InitCollectionChild();

	UpdateStyle();
}

void UNxActivatableWidgetWindow::InitLocalization()
{
	Super::InitLocalization();
}

void UNxActivatableWidgetWindow::UpdateStyle()
{
	for (auto itr : IncludeWidgets)
	{
		UCommonBorder* border = Cast<UCommonBorder>(itr.Value);
		if (nullptr != border)
		{
			auto s = border->Style;
			if (nullptr != s.Get())
			{
				FString Name = s.Get()->GetName();
				NxPrintWarning(LogUI, TEXT("CommonBorder Style Name : %s"), *Name);
			}
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

void	UNxActivatableWidgetWindow::SetPosition(FVector2D InPos)
{
	if (nullptr != RootCanvasPanel)
	{
		RootCanvasPanel->SetRenderTranslation(InPos);
	}
}

UPanelWidget* UNxActivatableWidgetWindow::GetSlot(const FString& InName)
{
	if (InName.IsEmpty())
		return RootCanvasPanel.Get();

	UWidget** widget = IncludeWidgets.Find(FName(*InName));
	if (nullptr == *widget)
		return nullptr;

	return Cast<UPanelWidget>((*widget));
}

void	UNxActivatableWidgetWindow::SetSize(FVector2D InSize)
{
	//SetDesiredSizeInViewport(InSize);

	if (nullptr != RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(InSize.X);
		RootSizeBox->SetHeightOverride(InSize.Y);
	}
}

#define LOCTEXT_NAMESPACE "VxLoc"

#if WITH_EDITOR

void UNxActivatableWidgetWindow::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// 함수 구현 체크
	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UNxActivatableWidgetWindow, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UNxActivatableWidgetWindow::StaticClass())
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

//FEventReply UVxUIBaseWnd::RootBorder_OnMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
//{
//	FEventReply Reply;
//	Reply.NativeReply = FReply::Unhandled();
//
//	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
//	{
//		if (true == bRootBorderClickClose)
//		{
//			CloseWnd();
//
//			Reply.NativeReply = FReply::Handled();
//		}
//	}
//
//	return Reply;
//}

#undef LOCTEXT_NAMESPACE
