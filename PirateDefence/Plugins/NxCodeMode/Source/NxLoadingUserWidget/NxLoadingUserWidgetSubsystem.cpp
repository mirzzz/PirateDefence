// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxLoadingUserWidgetSubsystem.h"
#include "NxLoadingUserWidgetModule..h"

#include "HAL/ThreadHeartBeat.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Framework/Application/IInputProcessor.h"
#include "PreLoadScreenManager.h"
#include "ShaderPipelineCache.h"
#include "Widgets/Images/SThrobber.h"
#include "Blueprint/UserWidget.h"

#include "NxLoadingUserWidget.h"
#include "NxLoadingUserWidgetSettings.h"
#include "NxLoadingUserWidgetProcessInterface.h"
#include "NxBaseLog.h"


//@TODO: Why can GetLocalPlayers() have nullptr entries?  Can it really?
//@TODO: Test with PIE mode set to simulate and decide how much (if any) loading screen action should occur
//@TODO: Allow other things implementing IVxLoadingProcessInterface besides GameState/PlayerController (and owned components) to register as interested parties
//@TODO: ChangeMusicSettings (either here or using the LoadingScreenVisibilityChanged delegate)
//@TODO: Studio analytics (FireEvent_PIEFinishedLoading / tracking PIE startup time for regressions, either here or using the LoadingScreenVisibilityChanged delegate)


// 로딩 화면의 프로파일링 카테고리 설정 (통계 캡쳐)
CSV_DEFINE_CATEGORY(_CsvLoading, true);

//-----------------------------------------------------------------------------------
bool INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
{
	if (TestObject != nullptr)
	{
		if (INxLoadingUserWidgetProcessInterface* loadObserver = Cast<INxLoadingUserWidgetProcessInterface>(TestObject))
		{
			FString observerReason; 

			if (loadObserver->ShouldShowLoadingScreen(/*out*/ observerReason))
			{
				if (ensureMsgf(!observerReason.IsEmpty(), TEXT("[%s] 에서 로드 화면에 표시할 내용이 비어 있습니다."), *GetPathNameSafe(TestObject)))
				{
					OutReason = observerReason;
				}
				return true;
			}
		}
	}

	return false;
}

// CVar
//-----------------------------------------------------------------------------------
namespace Nx_LoadingUserWidgetCVars
{
	// CVars
	static float HoldLoadingScreenAdditionalSecs = 2.0f;
	static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
		TEXT("Nx.LoadingUserWidget.HoldLoadingScreenAdditionalSecs"),
		HoldLoadingScreenAdditionalSecs,
		TEXT("텍스처 스트리밍이 흐릿함을 피할 기회를 주기 위해 다른 로딩이 끝난 후 로딩 화면을 유지하는 데 걸리는 시간 (float:2.0)"),
		ECVF_Default | ECVF_Preview);

	static bool LogLoadingScreenReasonEveryFrame = false;
	static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
		TEXT("Nx.LoadingUserWidget.LogLoadingScreenReasonEveryFrame"),
		LogLoadingScreenReasonEveryFrame,
		TEXT("로딩 화면이 표시되거나 숨겨지는 이유가 매 프레임마다 로그에 인쇄 (bool:false)"),
		ECVF_Default);

	static bool ForceLoadingScreenVisible = false;
	static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
		TEXT("Nx.LoadingUserWidget.AlwaysShow"),
		ForceLoadingScreenVisible,
		TEXT("로딩 화면을 강제로 표시 (bool:false)"),
		ECVF_Default);
}

// FNxLoadingUserWidgetInputPreProcessor
//-----------------------------------------------------------------------------------
// 로딩 화면이 표시될 때 적용할 입력 프로세서
// 모든 입력을 캡처하므로 로딩 화면 아래의 활성 메뉴는 상호 작용하지 않는다.
class FNxLoadingUserWidgetInputPreProcessor : public IInputProcessor
{
public:
	FNxLoadingUserWidgetInputPreProcessor() { }
	virtual ~FNxLoadingUserWidgetInputPreProcessor() { }

	bool CanEatInput() const
	{
		return !GIsEditor;
	}

	//~IInputProcess interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override { return CanEatInput(); }
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { return CanEatInput(); }
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanEatInput(); }
	//~End of IInputProcess interface
};


// UNxLoadingUserWidgetSubsystem
//-----------------------------------------------------------------------------------

bool UNxLoadingUserWidgetSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 클라이언트만 로딩 화면이 있습니다.
	const UGameInstance* gameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsClient = !gameInstance->IsDedicatedServerInstance();

	const UNxLoadingUserWidgetSettings* settings = GetDefault<UNxLoadingUserWidgetSettings>();
	const bool bIsWidget = settings->bUseWidgetLoading;

	return (bIsClient && bIsWidget);
}

void UNxLoadingUserWidgetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	NxPrintDisplay(LogLoading, TEXT("[----------------------------------------------------- NxLoadingUserWidgetSubsystem Initialize]"));

	Super::Initialize(Collection);

	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	HWorld_LevelAddedToWorld	 = FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::HandleLevelAddedToWorld);
	HWorld_LevelRemovedFromWorld = FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ThisClass::HandleLevelRemovedFromWorld);

	const UGameInstance* localGameInstance = GetGameInstance();
	check(localGameInstance);
}

void UNxLoadingUserWidgetSubsystem::Deinitialize()
{
	StopBlockingInput();

	RemoveWidgetFromViewport();

	FWorldDelegates::LevelAddedToWorld.Remove(HWorld_LevelAddedToWorld);
	FWorldDelegates::LevelRemovedFromWorld.Remove(HWorld_LevelRemovedFromWorld);

	FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
		
	NxPrintDisplay(LogLoading, TEXT("[--------------------------------------------------- NxLoadingUserWidgetSubsystem Deinitialize]"));
}

void UNxLoadingUserWidgetSubsystem::Tick(float DeltaTime)
{
	UpdateWidgetLoading();

	TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType UNxLoadingUserWidgetSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool UNxLoadingUserWidgetSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UNxLoadingUserWidgetSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNxLoadingUserWidgetSubsystem, STATGROUP_Tickables);
}

UWorld* UNxLoadingUserWidgetSubsystem::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

void UNxLoadingUserWidgetSubsystem::RegisterLoadingProcessor(TScriptInterface<INxLoadingUserWidgetProcessInterface> Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void UNxLoadingUserWidgetSubsystem::UnregisterLoadingProcessor(TScriptInterface<INxLoadingUserWidgetProcessInterface> Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

void UNxLoadingUserWidgetSubsystem::SetCustomLoadingLayout(FNxLoadingWidgetLayoutInfo& info)
{
	WidgetLoadingLayoutInfo = info;
}

void UNxLoadingUserWidgetSubsystem::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// 엔진이 초기화되면 즉시 로딩 화면 업데이트
		if (GEngine->IsInitialized())
		{
			UpdateWidgetLoading();
		}
	}
}

void UNxLoadingUserWidgetSubsystem::HandlePostLoadMap(UWorld* World)
{
	if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
	{
		bCurrentlyInLoadMap = false;
	}
}

void UNxLoadingUserWidgetSubsystem::HandleLevelAddedToWorld(ULevel* Level, UWorld* World)
{
	// 레벨 추가 이벤트
}

void UNxLoadingUserWidgetSubsystem::HandleLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
	// 레벨 제거 이벤트
}

void UNxLoadingUserWidgetSubsystem::UpdateWidgetLoading()
{
	bool bLogLoadingScreenStatus = Nx_LoadingUserWidgetCVars::LogLoadingScreenReasonEveryFrame;

	if (ShouldShowCustomLoading())
	{
		const UNxLoadingUserWidgetSettings* settings = GetDefault<UNxLoadingUserWidgetSettings>();
		
		// 지정된 시간 내에 지정된 체크포인트에 도달하지 않으면 중단 감지기가 작동하여 진행이 중단된 위치를 더 잘 파악할 수 있습니다.
 		FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), settings->WidgetLoadingSettings.LoadingHeartbeatHangDuration);

		ShowWidgetLoading();

 		if ((settings->WidgetLoadingSettings.LogLoadingHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
 		{
			bLogLoadingScreenStatus = true;
 			TimeUntilNextLogHeartbeatSeconds = settings->WidgetLoadingSettings.LogLoadingHeartbeatInterval;
 		}
	}
	else
	{
		HideWidgetLoading();
 
 		FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
	}

	if (bLogLoadingScreenStatus)
	{
		NxPrintLog(LogLoading, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingCustomLoading ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
	}
}

// 로딩 화면을 표시할 필요가 있는지 확인
bool UNxLoadingUserWidgetSubsystem::CheckForAnyNeedToShowCustomLoading()
{
	// 나중에 변경할 때 이유를 입력하는 것을 잊어버린 경우를 대비하여 '알 수 없는' 이유부터 시작.
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("LoadingScreen 을 표시하거나 숨기는 이유를 알 수 없습니다!");

	const UGameInstance* LocalGameInstance = GetGameInstance();

	if (Nx_LoadingUserWidgetCVars::ForceLoadingScreenVisible)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("VxLoadingScreen.AlwaysShow is true"));
		return true;
	}

	const FWorldContext* Context = LocalGameInstance->GetWorldContext();
	if (Context == nullptr)
	{
		// 월드 컨텍스트가 없을때 로딩 화면을 표시.
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
		return true;
	}

	UWorld* world = Context->World();
	if (world == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
		return true;
	}

	AGameStateBase* gameState = world->GetGameState<AGameStateBase>();
	if (gameState == nullptr)
	{
		// GameState 가 아직 복제되지 않음.
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
		return true;
	}

	if (bCurrentlyInLoadMap)
	{
		// LoadMap에 있는 경우 로딩 화면 표시
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
		return true;
	}

	if (!Context->TravelURL.IsEmpty())
	{
		// Travel 보류 시 로딩 화면 표시
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
		return true; 
	}

	if (Context->PendingNetGame != nullptr)
	{ 
		// 다른 서버에 연결 시 로딩 화면 표시
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
		return true;
	}

	if (!world->HasBegunPlay())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
		return true;
	}

	if (world->IsInSeamlessTravel())
	{
		// Show a loading screen during seamless travel
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
		return true;
	}

	// 로딩 화면이 필요한지 게임 상태에 묻기
	if (INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(gameState, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
	{
		return true;
	}

	// 로딩 화면이 필요한지 게임 상태 구성요소에 묻기.
	for (UActorComponent* TestComponent : gameState->GetComponents())
	{
		if (INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 등록되었을 수 있는 외부 로딩 프로세서 확인.
	// 스트리밍이 완료되는 동안 로딩 화면을 계속 표시하도록 게임 코드에 의해 등록된 액터 또는 구성 요소에서 확인.
	for (const TWeakInterfacePtr<INxLoadingUserWidgetProcessInterface>& Processor : ExternalLoadingProcessors)
	{
		if (INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(Processor.GetObject(), /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// local player 확인
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// 로딩 화면이 필요한지 playerController 에서 확인
				if (INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(PC, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
				{
					return true;
				}

				// 로딩 화면이 필요한지 playerController Component 에서 확인
				for (UActorComponent* TestComponent : PC->GetComponents())
				{
					if (INxLoadingUserWidgetProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
					{
						return true;
					}
				}
			}
			else
			{
				bMissingAnyLocalPC = true;
			}
		}
	}

	UGameViewportClient* gameViewportClient = LocalGameInstance->GetGameViewportClient();
	if (gameViewportClient)
	{
		const bool bIsInSplitscreen = gameViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;

		// 분할 화면에서는 모든 플레이어 컨트롤러가 있어야 합니다.
		if (bIsInSplitscreen && bMissingAnyLocalPC)
		{
			DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
			return true;
		}

		// 비분할 화면에서는 최소한 하나의 플레이어 컨트롤러가 있어야 합니다.
		if (!bIsInSplitscreen && !bFoundAnyLocalPC)
		{
			DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
			return true;
		}
	}

	// 로딩 화면 해제.
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool UNxLoadingUserWidgetSubsystem::ShouldShowCustomLoading()
{
	const UNxLoadingUserWidgetSettings* settings = GetDefault<UNxLoadingUserWidgetSettings>();
	
	// 강제 디버깅 명령 확인 (명령라인에 'NoLoading' 있을 경우 LoadingScreen 출력 안함)
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoading"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoading'"));
		return false;
	}
#endif

	// 로딩 화면을 표시해야 하는지 확인
	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowCustomLoading();

	// 원하는 경우 로딩 화면을 조금 더 길게 유지
	bool bWantToForceShowLoadingScreen = false;

	if (bNeedToShowLoadingScreen)
	{
		// 로딩 화면 보여줌.
		TimeCustomLoadingLastDismissed = -1.0;
	}
	else
	{
		// 더 이상 화면을 표시할 *필요*는 없지만 잠시 동안 표시하고 싶을 수도 있음		
		const double currentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || settings->WidgetLoadingSettings.HoldLoadingAdditionalSecsEvenInEditor);
		const double holdLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? Nx_LoadingUserWidgetCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeCustomLoadingLastDismissed < 0.0)
		{
			TimeCustomLoadingLastDismissed = currentTime;
		}
		const double TimeSinceScreenDismissed = currentTime - TimeCustomLoadingLastDismissed;

		// 스트리밍을 은폐하기 위해 추가로 X초 동안 기다린다.
		if ((holdLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < holdLoadingScreenAdditionalSecs))
		{
			// 현시점에서 월드를 렌더링하여 텍스처가 실제로 스트리밍 되는지 확인하세요.
			// @TODO: 이 기간 동안 bNeedToShowCustomLoading이 다시 true로 되돌아오면 이 기능을 다시 끄지 않습니다...
			UGameViewportClient* gameViewportClient = GetGameInstance()->GetGameViewportClient();
			gameViewportClient->bDisableWorldRendering = false;

			DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), holdLoadingScreenAdditionalSecs);
			bWantToForceShowLoadingScreen = true;
		}
	}

	return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool UNxLoadingUserWidgetSubsystem::IsShowingInitialCustomLoading() const
{
	FPreLoadScreenManager* preLoadScreenManager = FPreLoadScreenManager::Get();
	return (preLoadScreenManager != nullptr) && preLoadScreenManager->HasValidActivePreLoadScreen();
}

void UNxLoadingUserWidgetSubsystem::ShowWidgetLoading()
{
	// 현재 보여주고 있으면 넘김
	if (bCurrentlyShowingCustomLoading == true)
		return;

	// 엔진이 로딩 화면으로 로딩 중인 경우 로딩 화면을 표시할 수 없습니다.
	if (FPreLoadScreenManager::Get() 
		&& FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen))
	{
		return;
	}

	TimeCustomLoadingShown = FPlatformTime::Seconds();

	bCurrentlyShowingCustomLoading = true;

	CSV_EVENT(_CsvLoading, TEXT("Show"));

	const UNxLoadingUserWidgetSettings* settings = GetDefault<UNxLoadingUserWidgetSettings>();

	if (IsShowingInitialCustomLoading())
	{
		NxPrintLog(LogLoading, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		NxPrintLog(LogLoading, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		NxPrintLog(LogLoading, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		NxPrintLog(LogLoading, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UGameInstance* localGameInstance = GetGameInstance();

		// 로딩 화면시 입력 블럭.
		StartBlockingInput();

		// 로딩 화면 변경 이벤트 전달
		OnLoadingVisibilityChanged.Broadcast(/*bIsVisible=*/ true);

		// 로딩 화면 위젯 만들기
		TSubclassOf<UUserWidget> loadingWidgetClass = settings->WidgetLoadingSettings.LoadingWidget.TryLoadClass<UUserWidget>();
		if (UUserWidget* userWidget = UUserWidget::CreateWidgetInstance(*localGameInstance, loadingWidgetClass, NAME_None))
		{
			if(UNxLoadingUserWidget* loadingUserWidget = Cast<UNxLoadingUserWidget>(userWidget) )
			{
				// 로딩화면 적용
				loadingUserWidget->SetWidgetLayout(WidgetLoadingLayoutInfo);

				LoadingWidget = userWidget->TakeWidget();
			}
		}
		else
		{
			NxPrintError(LogLoading, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *settings->WidgetLoadingSettings.LoadingWidget.ToString());

			LoadingWidget = SNew(SThrobber);
		}

		// 높은 ZOrder로  뷰포트에 추가하여 대부분의 항목 위에 있도록 한다.
		UGameViewportClient* gameViewportClient = localGameInstance->GetGameViewportClient();
		gameViewportClient->AddViewportWidgetContent(LoadingWidget.ToSharedRef(), settings->WidgetLoadingSettings.LoadingZOrder);

		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ true);

		if (!GIsEditor || settings->WidgetLoadingSettings.ForceTickCustomLoadingEvenInEditor)
		{
			// 슬레이트 Tick()을 통해 로딩 화면이 즉시 표시됩니다.
			FSlateApplication::Get().Tick();
		}
	}
}

void UNxLoadingUserWidgetSubsystem::HideWidgetLoading()
{
	if (!bCurrentlyShowingCustomLoading)
	{
		return;
	}

	StopBlockingInput();

	if (IsShowingInitialCustomLoading())
	{
		NxPrintLog(LogLoading, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		NxPrintLog(LogLoading, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		NxPrintLog(LogLoading, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		NxPrintLog(LogLoading, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		NxPrintLog(LogLoading, TEXT("Garbage Collecting before dropping load screen"));
		GEngine->ForceGarbageCollection(true);

		RemoveWidgetFromViewport();
	
		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ false);

		// 관찰자에게 로딩 화면이 완료되었음을 알립니다.
		OnLoadingVisibilityChanged.Broadcast(/*bIsVisible=*/ false);
	}

	CSV_EVENT(_CsvLoading, TEXT("Hide"));

	const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeCustomLoadingShown;
	NxPrintLog(LogLoading, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);

	bCurrentlyShowingCustomLoading = false;
}

void UNxLoadingUserWidgetSubsystem::RemoveWidgetFromViewport()
{
	UGameInstance* localGameInstance = GetGameInstance();
	if (LoadingWidget.IsValid())
	{
		if (UGameViewportClient* gameViewportClient = localGameInstance->GetGameViewportClient())
		{
			gameViewportClient->RemoveViewportWidgetContent(LoadingWidget.ToSharedRef());
		}
		LoadingWidget.Reset();
	}
}

void UNxLoadingUserWidgetSubsystem::StartBlockingInput()
{
	if (!InputPreProcessor.IsValid())
	{
		// 입력 관리자 설정.
		InputPreProcessor = MakeShareable<FNxLoadingUserWidgetInputPreProcessor>(new FNxLoadingUserWidgetInputPreProcessor());
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void UNxLoadingUserWidgetSubsystem::StopBlockingInput()
{
	if (InputPreProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		InputPreProcessor.Reset();
	}
}

void UNxLoadingUserWidgetSubsystem::ChangePerformanceSettings(bool bEnabingLoadingScreen)
{
	UGameInstance* localGameInstance = GetGameInstance();
	UGameViewportClient* gameViewportClient = localGameInstance->GetGameViewportClient();

	FShaderPipelineCache::SetBatchMode(bEnabingLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

	// 로드하는 World Rendering Disable.
	gameViewportClient->bDisableWorldRendering = bEnabingLoadingScreen;

	// 로딩 화면이 켜져 있는 경우 레벨 스트리밍을 우선 순위로 지정.
	if (UWorld* viewportWorld = gameViewportClient->GetWorld())
	{
		if (AWorldSettings* worldSettings = viewportWorld->GetWorldSettings(false, false))
		{
			worldSettings->bHighPriorityLoadingLocal = bEnabingLoadingScreen;
		}
	}

	if (bEnabingLoadingScreen)
	{
		// 로딩화면 노출시, 새로운 정지 감지 설정.
		double hangDurationMultiplier;
		if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), /*out*/ hangDurationMultiplier, GEngineIni))
		{
			hangDurationMultiplier = 1.0;
		}
		FThreadHeartBeat::Get().SetDurationMultiplier(hangDurationMultiplier);

		// 로딩화면시 끊김현상 보고 안함.
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// 되돌림.
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
