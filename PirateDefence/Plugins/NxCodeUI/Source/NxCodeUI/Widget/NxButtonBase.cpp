// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxButtonBase.h"

#include "CommonActionWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/UMGSequencePlayer.h"
#include "MovieScene.h"	// MovieScene
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBoxSlot.h"

#include "Kismet/GameplayStatics.h"

#include "NxCodeUIUtility.h"

void UNxButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	CollectionChild();

	RefreshSizeMargin();
	RefreshButtonIcon();
	RefreshButtonText();
}

void UNxButtonBase::NativeConstruct()
{
	Super::NativeConstruct();

	OnClicked().AddUObject(this, &ThisClass::OnClickedEvent);
	OnDoubleClicked().AddUObject(this, &ThisClass::OnDoubleClickedEvent);
	OnPressed().AddUObject(this, &ThisClass::OnPressedEvent);
	OnReleased().AddUObject(this, &ThisClass::OnReleasedEvent);
	OnPressed().AddUObject(this, &ThisClass::OnPressedEvent);
	OnUnhovered().AddUObject(this, &ThisClass::OnUnhoveredEvent);
	OnFocusReceived().AddUObject(this, &ThisClass::OnFocusReceivedEvent);
	OnFocusLost().AddUObject(this, &ThisClass::OnFocusLostEvent);
}

void UNxButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
}

void UNxButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (nullptr != SoundIconHover)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundIconHover);
	}
}

void UNxButtonBase::NativeOnUnhovered()
{
	//todo

	Super::NativeOnUnhovered();
}

void UNxButtonBase::NativeOnClicked()
{
	Super::NativeOnClicked();

	if (nullptr != SoundIconClick)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundIconClick);
	}
}

void UNxButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	RefreshSizeMargin();
	RefreshButtonIcon();
	RefreshButtonText();
}

void UNxButtonBase::OnClickedEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::Clicked);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnDoubleClickedEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::DoubleClicked);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnPressedEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::Pressed);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnReleasedEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::Released);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnHoveredEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::Hovered);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnUnhoveredEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::Unhovered);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnFocusReceivedEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::FocusReceived);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::OnFocusLostEvent()
{
	auto info = GetAniInfo(E_ButtonEvent::FocusLost);
	if (nullptr != info)
	{
		PlayAnimation(FName(*info->Name), info->StartAtTime, info->NumberOfLoops, info->PlayMode);
	}
}

void UNxButtonBase::CollectionChild()
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

			// Widget 수집
			FString  widgetString = widget->GetName();
			IncludeWidgets.Add(FName(*widgetString), widget);
		}
	}
}

void UNxButtonBase::RefreshSizeMargin()
{
	if (nullptr != RootSizeBox)
	{
		if (bOverride_Size)
		{
			RootSizeBox->SetWidthOverride(ButtonSize.X);
			RootSizeBox->SetHeightOverride(ButtonSize.Y);
		}
	}

	if (bOverride_ButtonPadding)
	{
		SetPadding(ButtonPadding);
	}

	if (bOverride_RootBoxPadding)
	{
		if (nullptr != RootHorizontalBox)
			Nx_UtilityUI::SetPadding(RootHorizontalBox, RootBoxPadding);
		if (nullptr != RootVerticalBox)
			Nx_UtilityUI::SetPadding(RootVerticalBox, RootBoxPadding);
	}
}

FNxButtonAniInfo* UNxButtonBase::GetAniInfo(E_ButtonEvent ButtonEvent)
{
	for (int i=0; i<AniInfos.Num(); ++i)
	{
		if (ButtonEvent == AniInfos[i].ButtonEvent)
		{
			return &AniInfos[i];
		}
	}

	return nullptr;
}

void UNxButtonBase::SetButtonIcon(const FSlateBrush& InBrush, int InPos)
{
	if (InPos < ButtonIcons.Num())
	{
		ButtonIcons[InPos].Brush = InBrush;
	}

	RefreshButtonIcon();
}

void UNxButtonBase::SetButtonText(const FText& InText, int InPos)
{
	if (InPos < ButtonTexts.Num())
	{
		ButtonTexts[InPos].Text = InText;
	}

	RefreshButtonText();
}

// Animation
bool UNxButtonBase::IsExistAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
		return true;

	return false;
}

bool UNxButtonBase::IsPlayingAnimation(const FName& animationName)
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

UWidgetAnimation* UNxButtonBase::PlayAnimation(const FName& animationName, float startTime /*= 0.0f*/, int32 loopCount /*= 1*/, EUMGSequencePlayMode::Type playType /*= EUMGSequencePlayMode::Forward*/)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PlayAnimation(findAnim, startTime, loopCount, playType);
		CurrentWidgetAnimation = findAnim;
	}
	return findAnim;
}

UWidgetAnimation* UNxButtonBase::StopAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::StopAnimation(findAnim);
	}
	return findAnim;
}

UWidgetAnimation* UNxButtonBase::PauseAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PauseAnimation(findAnim);
	}
	return findAnim;
}

float UNxButtonBase::GetAnimation_CurrentTime(const FName& animationName)
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

float UNxButtonBase::GetAnimation_EndTime(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		return findAnim->GetEndTime();
	}
	return 0.0f;
}

void UNxButtonBase::GetAnimationList(OUT TArray<FName>& animationList)
{
	animationList.Empty();

	for (auto& ani : IncludeAnimations)
	{
		animationList.Add(ani.Key);
	}
}

void UNxButtonBase::WidgetAnimationStarted()
{
}

void UNxButtonBase::WidgetAnimationFinished()
{
	CurrentWidgetAnimation = nullptr;
}

