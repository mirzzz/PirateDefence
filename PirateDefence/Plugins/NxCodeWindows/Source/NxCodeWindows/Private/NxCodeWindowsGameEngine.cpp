
#include "NxCodeWindowsGameEngine.h"
#include "NxCodeWindowsModule.h"
#include "NxCodeWindowsManager.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealClient.h"
#include "NxBaseLog.h"


UNxCodeWindowsGameEngine::~UNxCodeWindowsGameEngine()
{

}

void UNxCodeWindowsGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);

	NxPrintFunc(LogCodeWindows);
}

void UNxCodeWindowsGameEngine::PreExit()
{
	NxPrintFunc(LogCodeWindows);

	Super::PreExit();

}

void UNxCodeWindowsGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	if (MultiWindowsManager == nullptr)
	{
		FNxCodeWindowsModule& multiWindowsModule = FModuleManager::LoadModuleChecked<FNxCodeWindowsModule>("NxCodeWindows");
		multiWindowsModule.GetMultiWindowsManager(MultiWindowsManager);
	}

	// 엔진 종료시 window 해제
	if (MultiWindowsManager && IsEngineExitRequested())
	{
		TArray<UNxCodeWindow*> subWindows = MultiWindowsManager->SubWindows;
		for (auto subWindow : subWindows)
		{
			if (subWindow == nullptr)
			{
				continue;
			}

			//  PIE 창 닫기 상자를 누른 경우 창은 이미 파괴되었을 수 있습니다.
			if (subWindow->GameViewportClientWindow.IsValid())
			{
				// SWindow를 삭제
				FSlateApplication::Get().DestroyWindowImmediately(subWindow->GameViewportClientWindow.Pin().ToSharedRef());
			}
		}
	}
		
	if (MultiWindowsManager)
	{
		MultiWindowsManager->UpdateWorldContentBeforeTick(WorldList);
	}
	
	Super::Tick(DeltaSeconds, bIdleMode);

	if (MultiWindowsManager)
	{
		MultiWindowsManager->UpdateWorldContentAfterTick(WorldList);
	}

	if (MultiWindowsManager)
	{
		TArray<UNxCodeWindow*> subWindows = MultiWindowsManager->SubWindows;
		for (auto subWindow : subWindows)
		{
			if (subWindow == nullptr)
				continue;

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
			//	MultiWindowsManager->SubWindows.Remove(Window);
			//	MultiWindowsManager->OnSubWindowsArrayChanged.Broadcast(MultiWindowsManager);
			//}

			if (MultiWindowsManager->bAddedNewWindow && subWindow && subWindow->GameViewportClient)
			{
				// AddNewWindow 후 처리할 부분 있으면 추가 작업
			}
		}
		MultiWindowsManager->bAddedNewWindow = false;
		
		
		for (auto subWindow : MultiWindowsManager->SubWindows)
		{
			if (subWindow == nullptr)
				continue;
			
			if (subWindow->GameViewportClient)
			{
				// Split Screen 조정 
				subWindow->GameViewportClient->LayoutPlayers();

				// 렌더링 진행
				if (subWindow->GameViewportClient->Viewport)
				{
					subWindow->GameViewportClient->Viewport->Draw();
				}

				NxPrintLog(LogCodeWindows, TEXT("Draw CodeWindowName: %s"), *subWindow->GetWindowTitle().ToString());
			}
		}
	}
}

