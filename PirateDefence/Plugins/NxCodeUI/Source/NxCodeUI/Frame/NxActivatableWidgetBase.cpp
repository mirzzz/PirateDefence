// Fill out your copyright notice in the Description page of Project Settings.


#include "NxActivatableWidgetBase.h"
#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Animation/UMGSequencePlayer.h"
#include "MovieScene.h"

#include "CommonBorder.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.h"

#include "NxBorderBase.h"
#include "Table/NxTableEnum.h"
#include "Localization/NxLocalizationSubsystem.h"



#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif 

const FName UNxActivatableWidgetBase::gAniName_Open			= TEXT("Open");
const FName UNxActivatableWidgetBase::gAniName_Close		= TEXT("Close");
const FName UNxActivatableWidgetBase::gAniName_Horizontal	= TEXT("HorizontalUI");
const FName UNxActivatableWidgetBase::gAniName_Vertical		= TEXT("VerticalUI");

UNxActivatableWidgetBase::UNxActivatableWidgetBase(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{

}

bool UNxActivatableWidgetBase::Initialize()
{
	if (false == Super::Initialize())
		return false;

	return true;
}

void UNxActivatableWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	InitCollectionChild();
	InitLocalization();
}

void UNxActivatableWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력키 핸들 등록
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		// 새로운 입력처리 활성화
		BeforeInputKeyHandler = ViewportClient->OnOverrideInputKey();
		ViewportClient->OnOverrideInputKey().BindUObject(this, &UNxActivatableWidgetBase::ProcessInputKey);
	}
}

void UNxActivatableWidgetBase::NativeDestruct()
{
	// 입력키 핸들 등록
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		// 되돌림.
		ViewportClient->OnOverrideInputKey() = BeforeInputKeyHandler;
		BeforeInputKeyHandler.Unbind();
	}

	Super::NativeDestruct();
}

void UNxActivatableWidgetBase::NativeOnActivated()
{
	//LogUI(Log);

	Super::NativeOnActivated();
}

void UNxActivatableWidgetBase::NativeOnDeactivated()
{
	//LogUI(Log);

	Super::NativeOnDeactivated();
}

bool UNxActivatableWidgetBase::Open(bool bHorizontal)
{
	PlayAnimation(gAniName_Open);
	PlayAnimation(bHorizontal ? gAniName_Horizontal : gAniName_Vertical);

	UpdateControl();

	return true;
}

void UNxActivatableWidgetBase::Close()
{
	PlayAnimation(gAniName_Close);

	DeactivateWidget();
}

void UNxActivatableWidgetBase::UpdateControl()
{
}

bool UNxActivatableWidgetBase::ProcessInputKey(FInputKeyEventArgs& EventArgs)
{
	return false;
}

TOptional<FUIInputConfig> UNxActivatableWidgetBase::GetDesiredInputConfig() const
{
	switch (InputMode)
	{
	case E_CommonInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case E_CommonInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case E_CommonInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	}

	return Super::GetDesiredInputConfig();
}

// Animation
bool UNxActivatableWidgetBase::IsExistAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
		return true;

	return false;
}

bool UNxActivatableWidgetBase::IsPlayingAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		// AnimationPlayer을 검색
		TObjectPtr<UUMGSequencePlayer>* findSequecne
			= ActiveSequencePlayers.FindByPredicate([&](const UUMGSequencePlayer* seqPlayer) { return seqPlayer->GetAnimation() == findAnim; });

		if (findSequecne != nullptr)
		{
			// Player 중이면 true 반환
			return (*findSequecne)->GetPlaybackStatus() == EMovieScenePlayerStatus::Playing;
		}
	}
	return false;
}

UWidgetAnimation* UNxActivatableWidgetBase::PlayAnimation(const FName& animationName, float startTime /*= 0.0f*/, int32 loopCount /*= 1*/, EUMGSequencePlayMode::Type playType /*= EUMGSequencePlayMode::Forward*/)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PlayAnimation(findAnim, startTime, loopCount, playType);
		CurrentWidgetAnimation = findAnim;
	}
	return findAnim;
}

UWidgetAnimation* UNxActivatableWidgetBase::StopAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::StopAnimation(findAnim);
	}
	return findAnim;
}

UWidgetAnimation* UNxActivatableWidgetBase::PauseAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PauseAnimation(findAnim);
	}
	return findAnim;
}

float UNxActivatableWidgetBase::GetAnimation_CurrentTime(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		// 활성화된 SequencePlayer중 Animation 이름과 같은 SequencePlayer 반환
		TObjectPtr<UUMGSequencePlayer>* findSequecne
			= ActiveSequencePlayers.FindByPredicate([&](const UUMGSequencePlayer* seqPlayer) { return seqPlayer->GetAnimation() == findAnim; });
		if (findSequecne != nullptr)
		{
			// Play 중인 현재 위치. (프레임 속도 컨텍스트에 의해 한정된 프레임 시간)
			FQualifiedFrameTime currentTime = (*findSequecne)->GetCurrentTime();
			return (float)currentTime.AsSeconds();
		}
	}
	return 0.0f;
}

float UNxActivatableWidgetBase::GetAnimation_EndTime(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		return findAnim->GetEndTime();
	}
	return 0.0f;
}

void UNxActivatableWidgetBase::GetAnimationList(OUT TArray<FName>& animationList)
{
	animationList.Empty();

	for (auto& ani : IncludeAnimations)
	{
		animationList.Add(ani.Key);
	}
}

void UNxActivatableWidgetBase::WidgetAnimationStarted()
{
}

void UNxActivatableWidgetBase::WidgetAnimationFinished()
{
	CurrentWidgetAnimation = nullptr;
}

void UNxActivatableWidgetBase::InitCollectionChild()
{
	// Blueprint Class 얻기.
	BPClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
	if (BPClass == nullptr)
		return;

	OnStartEvent.BindDynamic(this, &ThisClass::WidgetAnimationStarted);
	OnEndEvent.BindDynamic(this, &ThisClass::WidgetAnimationFinished);

	// BP Animation 등록.
	IncludeAnimations.Empty();
	for (auto& anim : BPClass->Animations)
	{
		UWidgetAnimation* animation = anim;
		if (animation != nullptr)
		{
			FString animString = animation->MovieScene->GetName();
			IncludeAnimations.Add(FName(*animString), animation);
			BindToAnimationStarted(animation, OnStartEvent);
			BindToAnimationFinished(animation, OnEndEvent);
		}
	}
	CurrentWidgetAnimation = nullptr;

	// Widget 등록.
	IncludeWidgets.Empty();

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

			UNxBorderBase* baseBorder = Cast<UNxBorderBase>(widget);
			if (nullptr != baseBorder)
				baseBorder->OnInitializedWidget(this);

			// Widget 수집
			FString  widgetString = widget->GetName();
			IncludeWidgets.Add(FName(*widgetString), widget);
		}
	}
}
void UNxActivatableWidgetBase::InitLocalization()
{
	//for (auto widget : IncludeWidgets)
	//{
	//	UTextBlock* textBlock = Cast<UTextBlock>(widget.Value);
	//	if (nullptr != textBlock)
	//	{
	//		FText findText = UVxLocalizationSubsystem::Get()->GetFromStringTable(E_StringTable::UI, textBlock->GetText().ToString());
	//		if(false == findText.IsEmpty())
	//		{
	//			textBlock->SetText(findText);
	//		}
	//	}

	//	//UTextBlock 에서 이미 처리
	//	//UCommonTextBlock* textBlock = Cast<UCommonTextBlock>(widget.Value);
	//	//if (nullptr != textBlock)
	//	//{
	//	//}
	//}
}

void UNxActivatableWidgetBase::RefreshSizeMargin()
{
	if (nullptr != RootSizeBox)
	{
		if (bOverride_Size)
		{
			RootSizeBox->SetWidthOverride(Widget_Size.X);
			RootSizeBox->SetHeightOverride(Widget_Size.Y);
		}
	}

	if (bOverride_Padding)
	{
		SetPadding(Widget_Padding);
	}
}

#define LOCTEXT_NAMESPACE "VxLoc"

#if WITH_EDITOR

void UNxActivatableWidgetBase::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// 함수 구현 체크
	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UNxActivatableWidgetBase, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UNxActivatableWidgetBase::StaticClass())
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
