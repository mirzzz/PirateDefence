// Fill out your copyright notice in the Description page of Project Settings.

#include "NxUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"

#include "Misc/QualifiedFrameTime.h"
#include "MovieScene.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

#include "NxActivatableWidgetWindow.h"
#include "NxUIEnum.h"

UNxUserWidget::UNxUserWidget()
{
	BPClass = nullptr;

	IncludeWidgets.Empty();
	IncludeAnimations.Empty();
}

bool UNxUserWidget::Initialize()
{
	if (Super::Initialize() == false)
		return false;

	return true;
}

void UNxUserWidget::RemoveFromParent()
{
	Super::RemoveFromParent();
}

void UNxUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	CollectionChild();

	RefreshSizeMargin();
}

void UNxUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNxUserWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNxUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	UUserWidget::NativeTick(MyGeometry, InDeltaTime);
}

FReply UNxUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FVector2D UNxUserWidget::GetMousePosition()
{
	FVector2D mousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	return mousePosition;

	// 카메라에서 바라보는 방향으로 위치 얻기.
	//APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0);
	//if (PC)
	//{
	//	// https://gall.dcinside.com/mgallery/board/view/?id=game_dev&no=17616
	//	FVector worldpos = FVector::ZeroVector;
	//	FVector worlddir = FVector::ZeroVector;
	//	if (PC->DeprojectMousePositionToWorld(worldpos, worlddir) == true)
	//	{
	//		// 마우스 위치와 카메라 방향을 이용하여 위치와 방향값 얻기.
	//	}
	//}
}

FVector2D UNxUserWidget::GetWidgetCenterPosition(UWidget* parentWidget, UWidget* widget)
{
	if (parentWidget == nullptr || widget == nullptr)
		return FVector2D::ZeroVector;

	// 절대 위치와 로컬 위치에 대한 이해 필요.
	FGeometry geometry = parentWidget->GetCachedGeometry();
	FVector2D position = geometry.AbsoluteToLocal(widget->GetCachedGeometry().GetAbsolutePosition());

	// Widget 크기의 반 (중간위치 값 얻기)
	position = position + widget->GetCachedGeometry().GetLocalSize() * 0.5f;

	return position;
}

// Animation
bool UNxUserWidget::IsExistAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
		return true;

	return false;
}

bool UNxUserWidget::IsPlayingAnimation(const FName& animationName)
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

UWidgetAnimation* UNxUserWidget::PlayAnimation(const FName& animationName, float startTime /*= 0.0f*/, int32 loopCount /*= 1*/, EUMGSequencePlayMode::Type playType /*= EUMGSequencePlayMode::Forward*/)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PlayAnimation(findAnim, startTime, loopCount, playType);
		CurrentWidgetAnimation = findAnim;
	}
	return findAnim;
}

UWidgetAnimation* UNxUserWidget::StopAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::StopAnimation(findAnim);
	}
	return findAnim;
}

UWidgetAnimation* UNxUserWidget::PauseAnimation(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		UUserWidget::PauseAnimation(findAnim);
	}
	return findAnim;
}

float UNxUserWidget::GetAnimation_CurrentTime(const FName& animationName)
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

float UNxUserWidget::GetAnimation_EndTime(const FName& animationName)
{
	UWidgetAnimation* findAnim = IncludeAnimations.FindRef(animationName);
	if (findAnim != nullptr)
	{
		return findAnim->GetEndTime();
	}
	return 0.0f;
}

void UNxUserWidget::GetAnimationList(OUT TArray<FName>& animationList)
{
	animationList.Empty();

	for (auto& ani : IncludeAnimations)
	{
		animationList.Add(ani.Key);
	}
}

void UNxUserWidget::WidgetAnimationStarted()
{
}

void UNxUserWidget::WidgetAnimationFinished()
{
	CurrentWidgetAnimation = nullptr;
}

void UNxUserWidget::CollectionChild()
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

void UNxUserWidget::RefreshSizeMargin()
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

void UNxUserWidget::Load(UNxActivatableWidgetWindow* InParentWnd)
{
	ParentWnd = InParentWnd;
}

UPanelWidget* UNxUserWidget::GetSlot(const FString& InName)
{
	if (InName.IsEmpty())
		return Panel_Root.Get();

	UWidget** widget = IncludeWidgets.Find(FName(*InName));
	if (nullptr == *widget)
		return nullptr;

	return Cast<UPanelWidget>((*widget));
}

void UNxUserWidget::CloseParentLayer()
{
	if (nullptr != ParentWnd)
	{
		ParentWnd.Get()->Close();
	}
}

void UNxUserWidget::SetVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}