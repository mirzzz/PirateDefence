
#include "NxCodeWindowsManager.h"

#include "Engine/Engine.h"
#include "Widgets/SOverlay.h"
#include "Slate/SGameLayerManager.h"
#include "Engine/GameEngine.h"
#include "Slate/SceneViewport.h"
#include "Framework/Application/SlateApplication.h"

#include "NxCodeWindowsGameViewportClient.h"
#include "NxCodeWindowsModule.h"
#include "NxBaseLog.h"

//void UNxCodeWindowsManager::CreateWindow(FText WindowTitle, FVector2D WindowPosition, FVector2D WindowSize)
//{
//	CreateWindow_Internal(WindowTitle, WindowPosition, WindowSize);
//}

TSharedPtr<SWindow> UNxCodeWindowsManager::CreateWindow_Internal(FText WindowTitle, FVector2D WindowPosition, FVector2D WindowSize)
{
	FText newWindowTitle = WindowTitle;

	if (newWindowTitle.IsEmpty())
	{
		newWindowTitle = FText::Format(NSLOCTEXT("NxCodeWindows", "NewWindow", "NewWindow-{Index}"), NumOfNewViewportWindow);
	}

	TSharedPtr<SWindow> newWindow = SNew(SWindow)
		.Title(newWindowTitle)
		.ScreenPosition(WindowPosition)
		.ClientSize(WindowSize)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.UseOSWindowBorder(true)
		.SaneWindowPlacement(false)
		.SizingRule(ESizingRule::UserSized);

	// Mac 은 부모 window 를 지원하지 않는다. do not keep on top
#if PLATFORM_MAC
	FSlateApplication::Get().AddWindow(newWindow.ToSharedRef());
#else
	// TSharedRef<SWindow, ESPMode::Fast> newWindow = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame")).GetParentWindow().ToSharedRef();
	FSlateApplication::Get().AddWindow(newWindow.ToSharedRef());
#endif

	NumOfNewViewportWindow++;
	return newWindow;
}

FSceneViewport* UNxCodeWindowsManager::GetSceneViewport(UGameViewportClient* GameViewportClient) const
{
	return GameViewportClient->GetGameViewport();
}

// 게임 뷰포트 생성.
void UNxCodeWindowsManager::CreateSceneViewport(UNxCodeWindow* Window)
{
	if (!Window || !Window->GameViewportClient)
	{
		return;
	}

	auto gameViewportClientWindow = Window->GameViewportClientWindow.Pin();

	if (!gameViewportClientWindow.IsValid())
	{
		return;
	}

	if (!Window->ViewportWidget.IsValid())
	{
		CreateViewportWidget(Window);
	}

	gameViewportClientWindow->SetOnWindowClosed(FOnWindowClosed::CreateUObject(Window, &UNxCodeWindow::OnGameWindowClosed));

	// SAVEWINPOS tells us to load/save window positions to user settings (this is disabled by default)
	int32 saveWinPos;
	if (GEngine && FParse::Value(FCommandLine::Get(), TEXT("SAVEWINPOS="), saveWinPos) && saveWinPos > 0)
	{
		// GameSettings에서 WinX/WinY 가져와서 설정하는 예제.
		//FIntPoint PiePosition = GEngine->GetGameUserSettings()->GetWindowPosition();
		//if (PiePosition.X >= 0 && PiePosition.Y >= 0)
		//{
		//	int32 WinX = GEngine->GetGameUserSettings()->GetWindowPosition().X;
		//	int32 WinY = GEngine->GetGameUserSettings()->GetWindowPosition().Y;
		//	gameViewportClientWindow->MoveWindowTo(FVector2D(WinX, WinY));
		//}
		//gameViewportClientWindow->SetOnWindowMoved(FOnWindowMoved::CreateUObject(Window, &UWindow::OnGameWindowMoved));
	}

	TSharedRef<SViewport> viewportWidgetRef = Window->ViewportWidget.ToSharedRef();

	TSharedPtr<class FSceneViewport> sceneViewport = MakeShareable(new FSceneViewport(Window->GameViewportClient, viewportWidgetRef));
	Window->SceneViewport = sceneViewport;
	Window->GameViewportClient->Viewport = sceneViewport.Get();

	// 마우스 기반 입력 시스템이 올바르게 작동할 때까지 비활성화됩니다.
	//GameViewportClient->CreateHighresScreenshotCaptureRegionWidget();

	// 뷰포트 위젯에는 무엇을 렌더링해야 하는지 알 수 있도록 인터페이스가 필요합니다.
	viewportWidgetRef->SetViewportInterface(sceneViewport.ToSharedRef());

	// FSlateApplication::Get().RegisterViewport(Window->ViewportWidget.ToSharedRef());

	FSceneViewport* viewportFrame = sceneViewport.Get();

	Window->GameViewportClient->SetViewportFrame(viewportFrame);

	Window->GameViewportClient->GetGameLayerManager()->SetSceneViewport(viewportFrame);

	gameViewportClientWindow->SetContent(viewportWidgetRef);

	// 뷰포트 크기 변경 이벤트 등록
	FViewport::ViewportResizedEvent.AddUObject(Window, &UNxCodeWindow::OnViewportResized);
}

void UNxCodeWindowsManager::CreateViewportWidget(UNxCodeWindow* Window)
{
	if (Window == nullptr)
		return;

	UGameViewportClient* gameViewportClient = Window->GameViewportClient;

	if (gameViewportClient == nullptr)
		return;

	bool bRenderDirectlyToWindow = true;

	TSharedRef<SOverlay> viewportOverlayWidgetRef = SNew(SOverlay);

	TSharedRef<SGameLayerManager> gameLayerManagerRef = 
		SNew(SGameLayerManager).SceneViewport_UObject(this, &UNxCodeWindowsManager::GetSceneViewport, gameViewportClient)
		[
			viewportOverlayWidgetRef
		];

	const bool bStereoAllowed = false;

	TSharedPtr<SViewport> viewportWidget =
		SNew(SViewport)
		// Movie 를 캡처하거나 스크린샷을 얻지 않는 한 Window의 백버퍼로 직접 렌더링합니다.
		// @todo TEMP
		.RenderDirectlyToWindow(bRenderDirectlyToWindow)
		// 게임의 감마 보정은 장면 렌더러의 후처리에서 처리됩니다.
		.EnableGammaCorrection(false)
		.EnableStereoRendering(bStereoAllowed)
		[
			gameLayerManagerRef
		];

	Window->ViewportWidget = viewportWidget;
	Window->GameViewportClient->SetViewportOverlayWidget(Window->GameViewportClientWindow.Pin(), viewportOverlayWidgetRef);
	Window->GameViewportClient->SetGameLayerManager(gameLayerManagerRef);
}

UNxCodeWindow* UNxCodeWindowsManager::CreateGameViewportClientWindow()
{
	if (GEngine == nullptr)
		return nullptr;

	UGameInstance* gameInstance = GEngine->GameViewport->GetGameInstance();
	if (gameInstance == nullptr)
		return nullptr;
	
	if (GIsClient == false)
		return nullptr;

	// 창 생성
	UNxCodeWindow* window = NewObject<UNxCodeWindow>(this, UNxCodeWindow::StaticClass());
	SubWindows.Add(window);

	// viewport client 초기화.
	UGameViewportClient* gameViewportClient = nullptr;
	gameViewportClient = NewObject<UGameViewportClient>(GEngine, GEngine->GameViewportClientClass);
	window->GameViewportClient = gameViewportClient;
	UNxCodeWindowsGameViewportClient* multiWindowsGameViewportClient = Cast<UNxCodeWindowsGameViewportClient>(gameViewportClient);
	if (multiWindowsGameViewportClient)
	{
		multiWindowsGameViewportClient->Window = window;
	}

	//FWorldContext& newWorldContext = GEngine->CreateNewWorldContext(EWorldType::Type::None);
	//newWorldContext.OwningGameInstance = gameInstance;

	FWorldContext& newWorldContext = GEngine->CreateNewWorldContext(gameInstance->GetWorldContext()->WorldType);
	FName contextHandle = newWorldContext.ContextHandle;
	FWorldContext* mainWorldContext = gameInstance->GetWorldContext();
	newWorldContext = *mainWorldContext;
	newWorldContext.ExternalReferences.Empty();
	newWorldContext.ContextHandle = contextHandle;
	newWorldContext.GameViewport = gameViewportClient;

	//UWorld* currentWorld;
	//GameInstance->GetWorldContext()->AddRef(currentWorld);
	//newWorldContext.SetCurrentWorld(currentWorld);
	//UWorld* World = UWorld::CreateWorld(newWorldContext.WorldType, false);
	//newWorldContext.SetCurrentWorld(World);

	gameViewportClient->Init(newWorldContext, gameInstance);
	bAddedNewWindow = true;

	// GameInstance->GetWorldContext()->GameViewport = GameViewportClient;

	// 뷰포트 클라이언트를 새 뷰포트에 연결합니다.
	if (gameViewportClient)
	{
		// 게임플레이 코드에서 위젯을 추가하기 전에 생성해야 합니다.
		TSharedPtr<SWindow> gameViewportClientWindow = CreateWindow_Internal(FText::FromString("NewGameViewportClientWindow"));

		window->GameViewportClientWindow = gameViewportClientWindow;
	
		CreateSceneViewport(window);

		FString errorMsg;
		if (gameViewportClient->SetupInitialLocalPlayer(errorMsg) == NULL)
		{
			NxPrintError(LogCodeWindows, TEXT("%s"), *errorMsg);			
		}

		UGameViewportClient::OnViewportCreated().Broadcast();
	}

	OnSubWindowsArrayChanged.Broadcast(this);
	return window;
}

void UNxCodeWindowsManager::GetMainWindow(UNxCodeWindow*& OutMainWindow)
{
	if (nullptr == MainWindow && GEngine && GEngine->GameViewport)
	{
		// 최상위 윈도우 생성
		MainWindow = NewObject<UNxCodeWindow>(this, UNxCodeWindow::StaticClass());
	}

	{
		MainWindow->GameViewportClient			= GEngine->GameViewport;
		MainWindow->GameViewportClientWindow	= GEngine->GameViewport->GetWindow();

		// MainWindow->SceneViewport = MakeShareable(GEngine->GameViewport->GetGameViewport());
		MainWindow->ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();

		UNxCodeWindowsGameViewportClient* multiWindowsGameViewportClient = Cast<UNxCodeWindowsGameViewportClient>(MainWindow->GameViewportClient);
		if (multiWindowsGameViewportClient)
		{
			multiWindowsGameViewportClient->Window = MainWindow;
		}
	}

	OutMainWindow = this->MainWindow;
}

// 엔진 업데이트 전 처리
void UNxCodeWindowsManager::UpdateWorldContentBeforeTick(TIndirectArray<FWorldContext>& WorldList)
{
	if (GEngine == nullptr)
		return;

	if (GEngine->GameViewport == nullptr)
		return;

	UGameInstance* gameInstance = GEngine->GameViewport->GetGameInstance();
	if (gameInstance == nullptr)
		return;

	FWorldContext* mainWorldContext = gameInstance->GetWorldContext();
	if (mainWorldContext == nullptr)
		return;

	if(mainWorldContext->TravelURL.IsEmpty() == false)
	{
		NxPrintLog(LogCodeWindows, TEXT("When Level Change start, TravelURL string : %s"), *mainWorldContext->TravelURL);

		//TempWorldList.Empty();
		TempGameViewportClients.Empty();

		for (int32 WorldIdx = WorldList.Num() - 1; WorldIdx >= 0; --WorldIdx)
		{
			FWorldContext& thisContext = WorldList[WorldIdx];

			for (int32 WindowIndex = SubWindows.Num()-1; WindowIndex >= 0; --WindowIndex)
			{
				UNxCodeWindow* subWindow = SubWindows[WindowIndex];
				if (subWindow)
				{
					// Level 변경 발생시, 현재 모듈에서 만들어진 ViewportClient인 경우 Temp로 이동시키고, WorldContextList에서 제거.
					if (thisContext.GameViewport == subWindow->GameViewportClient && subWindow->GameViewportClient )
					{
						thisContext.SetCurrentWorld(nullptr);
						//TempWorldList.Add(&ThisContext);
						TempGameViewportClients.Add(subWindow->GameViewportClient);
						WorldList.RemoveAt(WorldIdx);
					}
				}
			}
		}
	}
}

// 엔진 업데이트 후 처리
void UNxCodeWindowsManager::UpdateWorldContentAfterTick(TIndirectArray<FWorldContext>& WorldList)
{
	if (GEngine == nullptr)
		return;

	if (GEngine->GameViewport == nullptr)
		return;

	UGameInstance* gameInstance = GEngine->GameViewport->GetGameInstance();
	if (gameInstance == nullptr)
		return;

	FWorldContext* mainWorldContext = gameInstance->GetWorldContext();
	if (mainWorldContext == nullptr)
		return;

	if(TempGameViewportClients.Num())
	{
		NxPrintLog(LogCodeWindows, TEXT("When Level Change finish, TravelURL string : %s"), *mainWorldContext->TravelURL);

		// Level 변경 발생 시, 창을 유지하기 위해 내용이 있을 경우, NewWorldContext에 재 할당.
		for (int32 WorldIdx = TempGameViewportClients.Num() - 1; WorldIdx >= 0; --WorldIdx)
		{
			UGameViewportClient* thisGameViewportClient = TempGameViewportClients[WorldIdx];

			for (int32 WindowIndex = SubWindows.Num() - 1; WindowIndex >= 0; --WindowIndex)
			{
				UNxCodeWindow* subWindow = SubWindows[WindowIndex];
				if (subWindow)
				{
					if (thisGameViewportClient == subWindow->GameViewportClient && subWindow->GameViewportClient)
					{
						FWorldContext& newWorldContext = GEngine->CreateNewWorldContext(mainWorldContext->WorldType);
						newWorldContext.SetCurrentWorld(nullptr);

						FName contextHandle = newWorldContext.ContextHandle;
						newWorldContext = *mainWorldContext;
						newWorldContext.ExternalReferences.Empty();
						newWorldContext.ContextHandle = contextHandle;
						newWorldContext.GameViewport  = subWindow->GameViewportClient;

						subWindow->GameViewportClient->Init(newWorldContext, gameInstance);
						TempGameViewportClients.RemoveAt(WorldIdx);
					}
				}
			}
		}
		//TempWorldList.Empty();
	}
}

