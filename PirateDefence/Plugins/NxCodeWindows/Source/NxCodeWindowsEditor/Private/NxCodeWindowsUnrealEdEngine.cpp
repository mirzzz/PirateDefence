
#include "NxCodeWindowsUnrealEdEngine.h"
#include "NxCodeWindowsModule.h"
#include "NxCodeWindowsManager.h"
#include "Framework/Application/SlateApplication.h"

DEFINE_LOG_CATEGORY_STATIC(LogCodeWindowsEdEngine, Log, All);

UNxCodeWindowsUnrealEdEngine::~UNxCodeWindowsUnrealEdEngine()
{

}

void UNxCodeWindowsUnrealEdEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

void UNxCodeWindowsUnrealEdEngine::PreExit()
{
	Super::PreExit();
}

void UNxCodeWindowsUnrealEdEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	if (!CodeWindowsManager)
	{
		FNxCodeWindowsModule& multiWindowsModule = FModuleManager::LoadModuleChecked<FNxCodeWindowsModule>("NxCodeWindows");
		multiWindowsModule.GetMultiWindowsManager(CodeWindowsManager);
	}

	// EndPlayMap 종료시 window 해제
	if (CodeWindowsManager && bRequestEndPlayMapQueued)
	{
		TArray<UNxCodeWindow*> subWindows = CodeWindowsManager->SubWindows;
		for (auto subWindow : subWindows)
		{
			if (subWindow == nullptr)
			{
				continue;
			}

			// PIE 창 닫기 상자를 누른 경우 창은 이미 파괴되었을 수 있습니다.
			if (subWindow->GameViewportClientWindow.IsValid())
			{
				// SWindow를 삭제
				FSlateApplication::Get().DestroyWindowImmediately(subWindow->GameViewportClientWindow.Pin().ToSharedRef());
			}
		}
	}

	if (CodeWindowsManager)
	{
		CodeWindowsManager->UpdateWorldContentBeforeTick(WorldList);
	}

	Super::Tick(DeltaSeconds, bIdleMode);

	if (CodeWindowsManager)
	{
		CodeWindowsManager->UpdateWorldContentAfterTick(WorldList);
	}

	if (CodeWindowsManager)
	{
		TArray<UNxCodeWindow*> subWindows = CodeWindowsManager->SubWindows;
		for (auto& subWindow : subWindows)
		{
			if (subWindow == nullptr)
				continue;

			// 유효하지 않는 viewport 인 경우 해제.
			// 
			//bool IsValid = false;
			//for (int32 WorldIdx = WorldList.Num() - 1; WorldIdx >= 0; --WorldIdx)
			//{
			//	FWorldContext& ThisContext = WorldList[WorldIdx];
			//	if (ThisContext.GameViewport)
			//	{
			//		if (Window->GameViewportClient == ThisContext.GameViewport)
			//		{
			//			IsValid = true;
			//			break;
			//		}
			//	}
			//}
			//if (!IsValid)
			//{
			//	MultiWindowsManager->AncillaryWindows.Remove(Window);
			//  MultiWindowsManager->OnAncillaryWindowsArrayChanged.Broadcast(MultiWindowsManager);
			//}

			if (CodeWindowsManager->bAddedNewWindow && subWindow && subWindow->GameViewportClient)
			{
				// AddNewWindow 후 처리할 부분 있으면 추가 작업
			}
		}

		CodeWindowsManager->bAddedNewWindow = false;

		for (auto& subWindow : subWindows)
		{
			if (subWindow == nullptr)
				continue;

			if (subWindow->GameViewportClient)
			{
				// 렌더링.
				subWindow->GameViewportClient->LayoutPlayers();
				if (subWindow->GameViewportClient->Viewport)
				{
					subWindow->GameViewportClient->Viewport->Draw();
				}

				// UE_LOG(LogCodeWindowsEdEngine, Log, TEXT("UMultiWindowsUnrealEdEngine->Tick()"));
			}
		}
	}
}

