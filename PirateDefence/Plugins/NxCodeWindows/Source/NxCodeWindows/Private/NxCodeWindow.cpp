
#include "NxCodeWindow.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SGameLayerManager.h"
#include "Slate/SceneViewport.h"
#include "NxCodeWindowsLibrary.h"


void UNxCodeWindow::ClearViews()
{
	ViewManager.EnableMultiViews = true;
	ViewManager.Views.Empty();
}

void UNxCodeWindow::SetAsCustomViewMode(const FNxCodeViewManager& InViewManager)
{
	ViewManager = InViewManager;
}

void UNxCodeWindow::SetAsNormalViewMode()
{
	ViewManager.EnableMultiViews = false;
}

void UNxCodeWindow::ToggleNormalViewModeAndMultiViewsMode(const bool IsNormalViewMode)
{
	ViewManager.EnableMultiViews = !IsNormalViewMode;
}

void UNxCodeWindow::IsNormalViewModeOrMultiViewsMode(bool& IsNormalViewMode)
{
	IsNormalViewMode = !ViewManager.EnableMultiViews;
}

void UNxCodeWindow::GetWindowIndex(int32& WindowIndex, bool& IsValid)
{
	UNxCodeWindowsManager* multiWindowsManager;
	UNxCodeWindowsLibrary::GetMultiWindowsManager(multiWindowsManager);

	IsValid		= false;
	WindowIndex = INDEX_NONE;

	if (multiWindowsManager)
	{
		WindowIndex = multiWindowsManager->SubWindows.Find(this);
		if (WindowIndex != INDEX_NONE)
		{
			IsValid = true;
		}
	}
}

void UNxCodeWindow::ResizeWindow(int32 ResX, int32 ResY, EWindowMode::Type WindowMode)
{
	if (GameViewportClient == nullptr)
		return;

	// 변경 알림.
	FCoreDelegates::OnSystemResolutionChanged.Broadcast(ResX, ResY);

	//if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame)
	{
		FPlatformMisc::LowLevelOutputDebugStringf(TEXT("%s: SetRes 변경으로 인한 뷰포트 크기 조정, %d x %d\n"), *GetName(), ResX, ResY);
		GameViewportClient->ViewportFrame->ResizeFrame(ResX, ResY, WindowMode);
	}
}

void UNxCodeWindow::CloseWindow()
{
	if (GameViewportClientWindow.IsValid())
	{
		GameViewportClientWindow.Pin()->RequestDestroyWindow();
	}
}

void UNxCodeWindow::SetWindowTitle(FText WindowTitle)
{
	if (GameViewportClientWindow.Pin().IsValid())
	{
		GameViewportClientWindow.Pin()->SetTitle(WindowTitle);
	}
}

void UNxCodeWindow::SetWindowPosition(FVector2D NewPosition)
{
	if (GameViewportClientWindow.Pin().IsValid())
	{
		GameViewportClientWindow.Pin()->MoveWindowTo(NewPosition);
	}
}

void UNxCodeWindow::GetWindowPosition(FVector2D& WindowPosition)
{
	if (GameViewportClientWindow.Pin().IsValid())
	{
		WindowPosition = GameViewportClientWindow.Pin()->GetPositionInScreen();
	}
}

void UNxCodeWindow::GetWindowSize(FVector2D& WindowSize)
{
	if (GameViewportClientWindow.Pin().IsValid())
	{
		WindowSize = GameViewportClientWindow.Pin()->GetSizeInScreen();
	}
}

FText UNxCodeWindow::GetWindowTitle()
{
	if (GameViewportClientWindow.Pin().IsValid())
	{
		return GameViewportClientWindow.Pin()->GetTitle();
	}

	return FText();
}

void UNxCodeWindow::SetRectWindow(FVector2D WindowPosition, int32 ResX, int32 ResY, EWindowMode::Type WindowMode)
{
	ResizeWindow(ResX, ResY, WindowMode);
	SetWindowPosition(WindowPosition);
}

void UNxCodeWindow::OnGameWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	// FSlateApplication::Get().UnregisterGameViewport();
	//	This will shutdown the game
	if (SceneViewport->GetViewport() && GameViewportClient->Viewport)
	{
		GameViewportClient->CloseRequested(SceneViewport->GetViewport());
	}
	else
	{
		// 뷰포트 닫기 event 호출
		GameViewportClient->OnCloseRequested().Broadcast(SceneViewport->GetViewport());
		GameViewportClient->SetViewportFrame(NULL);

		TSharedPtr< IGameLayerManager > gameLayerManager(GameViewportClient->GetGameLayerManager());
		if (gameLayerManager.IsValid())
		{
			gameLayerManager->SetSceneViewport(nullptr);
		}
	}
	SceneViewport.Reset();

	TIndirectArray<FWorldContext>& worldList = const_cast<TIndirectArray<FWorldContext>&>(GEngine->GetWorldContexts());

	for (int32 Index= 0; Index < worldList.Num(); Index++)
	{
		FWorldContext& worldContext = worldList[Index];

		// 뷰포트가 지금 닫혀 있으면 PIE 세션을 종료합니다.
		if (worldContext.GameViewport != NULL && worldContext.GameViewport == GameViewportClient)
		{
			worldList.RemoveAt(Index);
		}
	}

	// 이 창의 뷰포트에서 모든 UserWidget을 제거합니다.
	TArray<UUserWidget*> userWidgetsInViewportTemp = UserWidgetsInViewport;
	for (auto userWidgetInViewport : userWidgetsInViewportTemp)
	{
		if (userWidgetInViewport)
		{
			TSharedRef<SWidget> UserSlateWidget = userWidgetInViewport->TakeWidget();
			TSharedPtr<SWidget> WidgetHost = UserSlateWidget->GetParentWidget();
			if (WidgetHost.IsValid())
			{
				// game World 인 경우,  현재 World 뷰포트에 위젯을 추가
				UWorld* World = userWidgetInViewport->GetWorld();
				if (World && World->IsGameWorld())
				{
					if (UGameViewportClient* viewportClient = GameViewportClient)
					{
						TSharedRef<SWidget> widgetHostRef = WidgetHost.ToSharedRef();

						viewportClient->RemoveViewportWidgetContent(widgetHostRef);

						if (ULocalPlayer* LocalPlayer = userWidgetInViewport->GetOwningLocalPlayer())
						{
							viewportClient->RemoveViewportWidgetForPlayer(LocalPlayer, widgetHostRef);
						}
					}
				}
			}
			else
			{
				userWidgetInViewport->RemoveFromParent();
			}

			UserWidgetsInViewport.Remove(userWidgetInViewport);
		}
	}
	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);

	UNxCodeWindowsManager* multiWindowsManager;
	UNxCodeWindowsLibrary::GetMultiWindowsManager(multiWindowsManager);

	if (multiWindowsManager)
	{
		multiWindowsManager->SubWindows.Remove(this);
		multiWindowsManager->OnSubWindowsArrayChanged.Broadcast(multiWindowsManager);
	}

	OnCloseWindow.Broadcast(this);
}

void UNxCodeWindow::OnGameWindowMoved(const TSharedRef<SWindow>& WindowBeingMoved)
{
	const FSlateRect WindowRect = WindowBeingMoved->GetRectInScreen();
	if (!GEngine)
	{
		return;
	}
	//GEngine->GetGameUserSettings()->SetWindowPosition(WindowRect.Left, WindowRect.Top);
	//GEngine->GetGameUserSettings()->SaveConfig();
}

void UNxCodeWindow::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	if (Viewport && Viewport == SceneViewport.Get() && GameViewportClientWindow.IsValid() && GameViewportClientWindow.Pin()->GetWindowMode() == EWindowMode::Windowed)
	{
		const FIntPoint viewportSize = Viewport->GetSizeXY();

		if (viewportSize.X > 0 && viewportSize.Y > 0)
		{
			//GSystemResolution.ResX = ViewportSize.X;
			//GSystemResolution.ResY = ViewportSize.Y;
			//FSystemResolution::RequestResolutionChange(GSystemResolution.ResX, GSystemResolution.ResY, EWindowMode::Windowed);

			//UGameUserSettings* Settings = GetGameUserSettings();
			//Settings->SetScreenResolution(ViewportSize);
			//Settings->ConfirmVideoMode();
			//Settings->RequestUIUpdate();
		}
	}
}

void UNxCodeWindow::OnLevelRemovedFromWorldAndRemoveWidgetsInViewport(ULevel* InLevel, UWorld* InWorld)
{
	// InLevel이 null 이면, 전 World 가 곧 사라질 것이라는 신호이므로 뷰포트에서 이 위젯을 제거 한다.
	// 다음 World 로 이어지지 않을 위험한 액터 참조를 너무 많이 잡고 있을 수 있다.
	if (InLevel == nullptr)
	{
		TArray<UUserWidget*> userWidgetsInViewportTemp = UserWidgetsInViewport;

		for (auto userWidgetInViewport : userWidgetsInViewportTemp)
		{
			if (userWidgetInViewport && InWorld == userWidgetInViewport->GetWorld())
			{
				TSharedRef<SWidget> userSlateWidget = userWidgetInViewport->TakeWidget();
				TSharedPtr<SWidget> widgetHost		= userSlateWidget->GetParentWidget();

				// 현재 GameWorld의 viewport 에 추가된 widget 이 있다면, 제거.
				UWorld* World = userWidgetInViewport->GetWorld();

				if (World && World->IsGameWorld())
				{
					if (GameViewportClient)
					{
						TSharedRef<SWidget> widgetHostRef = widgetHost.ToSharedRef();

						GameViewportClient->RemoveViewportWidgetContent(widgetHostRef);

						if (ULocalPlayer* localPlayer = userWidgetInViewport->GetOwningLocalPlayer())
						{
							GameViewportClient->RemoveViewportWidgetForPlayer(localPlayer, widgetHostRef);
						}
					}
				}

				userWidgetInViewport->RemoveFromParent();
				UserWidgetsInViewport.Remove(userWidgetInViewport);
			}
		}
		FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
	}
}