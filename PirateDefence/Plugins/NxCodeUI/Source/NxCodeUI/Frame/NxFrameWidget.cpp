// Fill out your copyright notice in the Description page of Project Settings.

#include "NxFrameWidget.h"
#include "NxActivatableWidgetWindow.h"
#include "NxCodeUILibrary.h"

#include "Components/CanvasPanel.h"

#include "NxCodeWindow.h"
#include "NxActivatableWidgetComponent.h"

#include "NxBaseLog.h"

UNxFrameWidget::UNxFrameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UNxFrameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxFrameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitLayer();
}

void UNxFrameWidget::Init(FVector2D InScreenSize, UNxCodeWindow* InVxWindow)
{
	ScreenSize = InScreenSize;
	MultiWindow = InVxWindow;
}

bool UNxFrameWidget::IsOpenUI(const FString& InUIName)
{
	for (auto& layer : Layers)
	{
		for (auto& wnd : layer->GetWnds())
		{
			if (InUIName == wnd.GetName())
				return true;
		}
	}

	return false;
}

UNxActivatableWidgetWindow* UNxFrameWidget::GetTopUI(E_StackLayer Layer)
{
	auto wnds = Layers[(int)Layer]->GetWnds();
	if (wnds.Num() <= 0)
		return nullptr;

	return wnds[0];
}

void UNxFrameWidget::RemoveWidgetFromLayer(UNxActivatableWidgetWindow* UIWnd)
{
	for (const auto& Layer : Layers)
	{
		Layer->RemoveWidget(*UIWnd);
	}
}

void UNxFrameWidget::CloseTopWnd(E_StackLayer Layer)
{
	UNxActivatableWidgetStack* WndLayer = GetLayer(Layer);
	auto wnds = WndLayer->GetWnds();
	if (wnds.Num() > 0)
	{
		wnds[0]->Close();
	}
}

void UNxFrameWidget::InitLayer()
{
	Layers.Empty();

	Layers.Add(Layer_Game);
	Layers.Add(Layer_GameMenu);
	Layers.Add(Layer_Menu);
	Layers.Add(Layer_Dialog);
	Layers.Add(Layer_Notices);
	Layers.Add(Layer_Indicator);
	Layers.Add(Layer_Dev);
}

UNxActivatableWidgetStack* UNxFrameWidget::GetLayer(E_StackLayer Layer)
{
	return Layers[(uint8)Layer];
}

void UNxFrameWidget::ClearLayer(E_StackLayer Layer)
{
	UNxActivatableWidgetStack* WndLayer = GetLayer(Layer);

	auto wnds = WndLayer->GetWnds();
	for(auto wnd : wnds)
		wnd->Close();
	if (nullptr != WndLayer && WndLayer->GetNumWidgets())
		WndLayer->ClearWidgets();
}

void UNxFrameWidget::RegisterLayer(UNxActivatableWidgetStack* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UNxFrameWidget::OnWidgetStackTransitioning);
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerWidget);
	}
}

void UNxFrameWidget::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UNxCodeUILibrary::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UNxCodeUILibrary::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void UNxFrameWidget::ViewportResized(FViewport* Viewport, uint32 Unused)
{
	//todo Frame별로 Event를 받아야 함..
	FVector2D Size = GetWindowSize();
	NxPrintDisplay(LogUI, TEXT("NxCodeUISubsystem::OnViewportResized() With[%d], Height[%d]"), Size.X, Size.Y);

	bool preScreenHorizontal = ScreenSize.X >= ScreenSize.Y ? true : false;
	bool nextScreenHorizontal = Size.X >= Size.Y ? true : false;
	if (preScreenHorizontal != nextScreenHorizontal)
	{
		ChangeScreenAspectRatio(nextScreenHorizontal);
	}

	ScreenSize = Size;
}

void UNxFrameWidget::ChangeScreenAspectRatio(bool Horizontal)
{
	//todo
	//Next를 향하여 Animation을 합시다. ~
	for (auto layer : Layers)
	{
		for (auto wnd : layer->GetWnds())
		{
			wnd->PlayAnimation(true == Horizontal ? UNxActivatableWidgetBase::gAniName_Horizontal : UNxActivatableWidgetBase::gAniName_Vertical);

			for (auto component : wnd->GetComponents())
			{
				component->PlayAnimation(true == Horizontal ? UNxActivatableWidgetBase::gAniName_Horizontal : UNxActivatableWidgetBase::gAniName_Vertical);
			}
		}
	}
}

FVector2D	UNxFrameWidget::GetWindowSize()
{
	FVector2D size;
	if (nullptr == MultiWindow)	//Main
	{
		if (GEngine && GEngine->GameViewport)
		{
			TWeakPtr<SWindow> window = GEngine->GameViewport->GetWindow();
			UGameViewportClient* viewport = GEngine->GameViewport;
			if (nullptr != viewport)
			{
				FIntPoint viewSize = viewport->Viewport->GetSizeXY();
				size.Set(viewSize.X, viewSize.Y);
			}
		}
	}
	else //Sub
	{
		auto window = MultiWindow.Get();
		window->GetWindowSize(size);
	}

	return size;
}

bool		UNxFrameWidget::IsWindowHorizontal()
{
	return ScreenSize.X >= ScreenSize.Y ? true : false;
}

UNxCodeWindow* UNxFrameWidget::GetMultiWindow()
{
	return MultiWindow.Get();
}