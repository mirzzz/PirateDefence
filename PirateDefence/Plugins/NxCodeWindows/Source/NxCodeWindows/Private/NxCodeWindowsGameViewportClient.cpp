
#include "NxCodeWindowsGameViewportClient.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"
#include "GameMapsSettings.h"
#include "EngineStats.h"
#include "RenderingThread.h"
#include "SceneView.h"
#include "AudioDeviceHandle.h"
#include "LegacyScreenPercentageDriver.h"
#include "AI/NavigationSystemBase.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "GameFramework/Volume.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Package.h"
#include "SceneManagement.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/NetDriver.h"
#include "Engine/LocalPlayer.h"
#include "ContentStreaming.h"
#include "UnrealEngine.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "Engine/Console.h"
#include "GameFramework/HUD.h"
#include "FXSystem.h"
#include "SubtitleManager.h"
#include "ImageUtils.h"
#include "SceneViewExtension.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "EngineModule.h"
#include "AudioDeviceManager.h"
#include "AudioDevice.h"
#include "Audio/AudioDebug.h"
#include "Sound/SoundWave.h"
#include "HighResScreenshot.h"
#include "BufferVisualizationData.h"
#include "GameFramework/InputSettings.h"
#include "Components/LineBatchComponent.h"
#include "Debug/DebugDrawService.h"
#include "Components/BrushComponent.h"
#include "Engine/GameEngine.h"
#include "Logging/MessageLog.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/UserInterfaceSettings.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"
#include "ActorEditorUtils.h"
#include "ComponentRecreateRenderStateContext.h"
#include "DynamicResolutionState.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "StereoRendering.h"

#include "NxCodeWindowsLocalPlayer.h"
#include "NxBaseLog.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "Engine/DebugCameraController.h"
#endif


CSV_DEFINE_CATEGORY(View_MultiWindowsGameViewportClient, true);

#define LOCTEXT_NAMESPACE "NxCodeWindowsGameViewportClient"


static TAutoConsoleVariable<int32> CVarSetBlackBordersEnabled( 
	TEXT("Vx.MutltiWindow.BlackBorders"),
	0,
	TEXT("To draw black borders around the rendered image\n")
	TEXT("(prevents artifacts from post processing passes that read outside of the image e.g. PostProcessAA)\n")
	TEXT("in pixels, 0:off"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarScreenshotDelegate(
	TEXT("Vx.MutltiWindow.ScreenshotDelegate"),
	1,
	TEXT("ScreenshotDelegates prevent processing of incoming screenshot request and break some features. This allows to disable them.\n")
	TEXT("Ideally we rework the delegate code to not make that needed.\n")
	TEXT(" 0: off\n")
	TEXT(" 1: delegates are on (default)"),
	ECVF_Default);

// TODO: 사용자 설정으로 만들어야 할 필요가 있나?
static TAutoConsoleVariable<float> CVarSecondaryScreenPercentage( 
	TEXT("Vx.MutltiWindow.SecondaryScreenPercentage.GameViewport"),
	0,
	TEXT("게임 뷰포트의 보조 화면 비율을 재정의합니다.\n")
	TEXT(" 0: 보조 화면 비율 계산 = 100 / DPI 배율 인자 기본값 사용됨\n")
	TEXT(" 1: 보조 화면 비율을 재정의."),
	ECVF_Default);


// Statistic -----------------------------------------

// UI stats
DECLARE_CYCLE_STAT(TEXT("UI Drawing Time"), STAT_UIDrawingTime_MultiWindowsGameViewportClient, STATGROUP_UI);
// HUD stat
DECLARE_CYCLE_STAT(TEXT("HUD Time"), STAT_HudTime_MultiWindowsGameViewportClient, STATGROUP_Engine);
// Engine stat
DECLARE_CYCLE_STAT(TEXT("CalcSceneView"), STAT_CalcSceneView_MultiWindowsGameViewportClient, STATGROUP_Engine);


// 임시 패키지에서 명명된 캔버스를 찾고 찾을 수 없는 경우 생성하는 유틸리티
static UCanvas* GetCanvasByName(FName CanvasName)
{
	// Cache to avoid FString/FName conversions/compares
	static TMap<FName, UCanvas*> CanvasMap;
	UCanvas** FoundCanvas = CanvasMap.Find(CanvasName);
	if (!FoundCanvas)
	{
		UCanvas* CanvasObject = FindObject<UCanvas>(GetTransientPackage(), *CanvasName.ToString());
		if (!CanvasObject)
		{
			CanvasObject = NewObject<UCanvas>(GetTransientPackage(), CanvasName);
			CanvasObject->AddToRoot();
		}

		CanvasMap.Add(CanvasName, CanvasObject);
		return CanvasObject;
	}

	return *FoundCanvas;
}

const int32 UNxCodeWindowsGameViewportClient::MaxNumOfViews = 20;

UNxCodeWindowsGameViewportClient::UNxCodeWindowsGameViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UNxCodeWindowsGameViewportClient::UNxCodeWindowsGameViewportClient(FVTableHelper& Helper)
	: Super(Helper)
{

}

UNxCodeWindowsGameViewportClient::~UNxCodeWindowsGameViewportClient()
{
}

void UNxCodeWindowsGameViewportClient::PostInitProperties()
{
	Super::PostInitProperties();

	ViewStates.SetNum(MaxNumOfViews);
	FSceneInterface* Scene = nullptr;
	UWorld* WorldTemp = GetWorld();
	if (WorldTemp)
	{
		Scene = WorldTemp->Scene;
	}
	for (auto& State : ViewStates)
	{
		State.Allocate(Scene ? Scene->GetFeatureLevel() : GMaxRHIFeatureLevel);
	}
}

void UNxCodeWindowsGameViewportClient::FinishDestroy()
{
	for (FSceneViewStateReference& ViewStateTemp : ViewStates)
	{
		ViewStateTemp.Destroy();
	}

	Super::FinishDestroy();
}

void UNxCodeWindowsGameViewportClient::UpdateCsvCameraStats(const FSceneView* View)
{
#if CSV_PROFILER
	if ( View == nullptr )
		return;

	static uint32 PrevFrameNumber = GFrameNumber;
	static double PrevTime = 0.0;
	static FVector PrevViewOrigin = FVector(ForceInitToZero);

	// TODO: 분할 화면 등 여러 보기/보기 계열을 지원합니다. 지금은 첫 번째 통계만 출력합니다.
	if (GFrameNumber != PrevFrameNumber)
	{
		FVector ViewOrigin = View->ViewMatrices.GetViewOrigin();
		FVector ForwardVec = View->ViewMatrices.GetOverriddenTranslatedViewMatrix().GetColumn(2);
		FVector UpVec = View->ViewMatrices.GetOverriddenTranslatedViewMatrix().GetColumn(1);
		FVector Diff = ViewOrigin - PrevViewOrigin;
		double CurrentTime = FPlatformTime::Seconds();
		double DeltaT = CurrentTime - PrevTime;
		FVector Velocity = Diff / float(DeltaT);
		float CameraSpeed = Velocity.Size();
		PrevViewOrigin = ViewOrigin;
		PrevTime = CurrentTime;
		PrevFrameNumber = GFrameNumber;

		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, PosX, View->ViewMatrices.GetViewOrigin().X, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, PosY, View->ViewMatrices.GetViewOrigin().Y, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, PosZ, View->ViewMatrices.GetViewOrigin().Z, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, ForwardX, ForwardVec.X, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, ForwardY, ForwardVec.Y, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, ForwardZ, ForwardVec.Z, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, UpX, UpVec.X, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, UpY, UpVec.Y, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, UpZ, UpVec.Z, ECsvCustomStatOp::Set);
		CSV_CUSTOM_STAT(View_MultiWindowsGameViewportClient, Speed, CameraSpeed, ECsvCustomStatOp::Set);
	}
#endif
}

void UNxCodeWindowsGameViewportClient::GetViewPoint(class ULocalPlayer* LocalPlayer, const int32 IndexOfView, FMinimalViewInfo& OutViewInfo) const
{
	// if (FLockedViewState::Get().GetViewPoint(LocalPlayer, OutViewInfo.Location, OutViewInfo.Rotation, OutViewInfo.FOV) == false && PlayerController != NULL)
	
	if (!LocalPlayer)
	{
		return;
	}

	APlayerController* playerController = LocalPlayer->PlayerController;
	if (playerController != NULL)
	{
		if (playerController->PlayerCameraManager != NULL)
		{
			OutViewInfo = playerController->PlayerCameraManager->GetCameraCacheView();
			OutViewInfo.FOV = playerController->PlayerCameraManager->GetFOVAngle();
			playerController->GetPlayerViewPoint(/*out*/ OutViewInfo.Location, /*out*/ OutViewInfo.Rotation);
		}
		else
		{
			playerController->GetPlayerViewPoint(/*out*/ OutViewInfo.Location, /*out*/ OutViewInfo.Rotation);
		}
	}

	// 다른 클래스에서 ultra-wide 같은 종횡비로 조정될 수도 있으므로, 원래 원하는 FOV를 저장 합니다.
	OutViewInfo.DesiredFOV = OutViewInfo.FOV;

	FNxCodeView CurrentView = Window->ViewManager.Views[IndexOfView];
	if (CurrentView.ViewpointType == E_ViewPointType::CustomViewPoint)
	{
		CurrentView.CustomViewPoint.CustomPOV.CopyToViewInfo(OutViewInfo);
	}
	else if (CurrentView.ViewpointType == E_ViewPointType::BindToViewTarget)
	{
		CurrentView.BindToViewTarget.ApplyToViewInfo(OutViewInfo);
	}

	OffsetViewLocationAndRotation(OutViewInfo, IndexOfView);

	for (auto& ViewExt : GEngine->ViewExtensions->GatherActiveExtensions(FSceneViewExtensionContext(Viewport)))
	{
		ViewExt->SetupViewPoint(playerController, OutViewInfo);
	};
}

void UNxCodeWindowsGameViewportClient::OffsetViewLocationAndRotation(FMinimalViewInfo& InOutViewInfo, const int32 IndexOfView) const
{
	FNxCodeView CurrentView = Window->ViewManager.Views[IndexOfView];

	FVector ViewLocation = InOutViewInfo.Location;
	FRotator ViewRotation = InOutViewInfo.Rotation;

	const FTransform ViewRelativeTransform(CurrentView.RotationOffsetOfViewpoint, CurrentView.LocationOffsetOfViewpoint);
	const FTransform ViewWorldTransform(ViewRotation, ViewLocation);
	FTransform NewViewWorldTransform = ViewRelativeTransform * ViewWorldTransform;

	ViewLocation = NewViewWorldTransform.GetLocation();
	ViewRotation = NewViewWorldTransform.GetRotation().Rotator();

	InOutViewInfo.Location = ViewLocation;
	InOutViewInfo.Rotation = ViewRotation;
}

bool UNxCodeWindowsGameViewportClient::CalcSceneViewInitOptions(class ULocalPlayer* LocalPlayer,
	struct FSceneViewInitOptions& ViewInitOptions,
	FViewport* InViewport,
	const int32 IndexOfView,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	// CalcSceneViewInitOptions(ViewInitOptions, InViewport, ViewDrawer, StereoPass);

	if (LocalPlayer == nullptr)
		return false;

	FNxCodeView& view = Window->ViewManager.Views[IndexOfView];

	APlayerController* playerController = LocalPlayer->PlayerController;
	FVector2D size = LocalPlayer->Size;

	if ((playerController == NULL) || (size.X <= 0.f) || (size.Y <= 0.f) || (InViewport == NULL))
	{
		return false;
	}
	// 투영 데이터 가져오기
	if (LocalPlayer->GetProjectionData(InViewport, /*inout*/ ViewInitOptions, StereoViewIndex) == false)
	{
		// 필요한 정보를 얻지 못한 경우 NULL을 반환
		return false;
	}

	// 유효하지 않은 view rect일 경우 반환
	if (!ViewInitOptions.IsValidViewRectangle())
	{
		return false;
	}

	if (playerController->PlayerCameraManager != NULL)
	{
		// Screen에 화면 페이드 효과를 적용
		if (playerController->PlayerCameraManager->bEnableFading)
		{
			ViewInitOptions.OverlayColor = playerController->PlayerCameraManager->FadeColor;
			ViewInitOptions.OverlayColor.A = FMath::Clamp(playerController->PlayerCameraManager->FadeAmount, 0.0f, 1.0f);
		}

		// 원하는 경우 색상 스케일링을 수행
		if (playerController->PlayerCameraManager->bEnableColorScaling)
		{
			ViewInitOptions.ColorScale = FLinearColor(
				playerController->PlayerCameraManager->ColorScale.X,
				playerController->PlayerCameraManager->ColorScale.Y,
				playerController->PlayerCameraManager->ColorScale.Z
			);
		}

		// 카메라 cut 용 프레임인지 확인
		ViewInitOptions.bInCameraCut = playerController->PlayerCameraManager->bGameCameraCutThisFrame;
	}

	if (GEngine->StereoRenderingDevice.IsValid())
	{
		ViewInitOptions.StereoPass = GEngine->StereoRenderingDevice->GetViewPassForIndex(StereoViewIndex != INDEX_NONE, StereoViewIndex);
	}

	check(playerController && playerController->GetWorld());

	switch (ViewInitOptions.StereoPass)
	{
	case EStereoscopicPass::eSSP_FULL:
	case EStereoscopicPass::eSSP_PRIMARY:
		ViewInitOptions.SceneViewStateInterface = ViewStates[IndexOfView].GetReference();
		break;
	/*case EStereoscopicPass::eSSP_SECONDARY:
		ViewInitOptions.SceneViewStateInterface = ViewStates[IndexOfView].GetReference();
		break;*/
	}

	ViewInitOptions.ViewActor			= playerController->GetViewTarget();
	ViewInitOptions.PlayerIndex			= LocalPlayer->GetControllerId();
	ViewInitOptions.ViewElementDrawer	= ViewDrawer;
	ViewInitOptions.BackgroundColor		= FLinearColor::Black;
	ViewInitOptions.LODDistanceFactor	= playerController->LocalPlayerCachedLODDistanceFactor;
	ViewInitOptions.WorldToMetersScale	= playerController->GetWorldSettings()->WorldToMeters;
	ViewInitOptions.CursorPos = InViewport->HasMouseCapture() ? FIntPoint(-1, -1) : FIntPoint(InViewport->GetMouseX(), InViewport->GetMouseY());
	ViewInitOptions.OriginOffsetThisFrame = playerController->GetWorld()->OriginOffsetThisFrame;

	return true;
}

FSceneView* UNxCodeWindowsGameViewportClient::CalcSceneView(class ULocalPlayer* LocalPlayer,
	class FSceneViewFamily* ViewFamily,
	FVector& OutViewLocation,
	FRotator& OutViewRotation,
	FViewport* InViewport,
	int32 IndexOfView,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_CalcSceneView_MultiWindowsGameViewportClient);

	if (!LocalPlayer)
	{
		return nullptr;
	}

	FNxCodeView currentView = Window->ViewManager.Views[IndexOfView];

	FVector2D originCache = LocalPlayer->Origin;
	FVector2D sizeCache = LocalPlayer->Size;

	LocalPlayer->Origin = FVector2D(currentView.LocationAndSizeOnScreen.X, currentView.LocationAndSizeOnScreen.Y);
	LocalPlayer->Size = FVector2D(currentView.LocationAndSizeOnScreen.Z, currentView.LocationAndSizeOnScreen.W);

	FSceneViewInitOptions ViewInitOptions;

	if (!CalcSceneViewInitOptions(LocalPlayer, ViewInitOptions, InViewport, IndexOfView, ViewDrawer, StereoViewIndex))
	{
		return nullptr;
	}

	// Get the viewpoint... 기술적으로 두 번 실행하지만 GetProjectionData 가 더 좋습니다.
	FMinimalViewInfo viewInfo;
	GetViewPoint(LocalPlayer, IndexOfView, viewInfo);
	OutViewLocation = viewInfo.Location;
	OutViewRotation = viewInfo.Rotation;
	ViewInitOptions.bUseFieldOfViewForLOD = viewInfo.bUseFieldOfViewForLOD;
	ViewInitOptions.FOV = viewInfo.FOV;
	ViewInitOptions.DesiredFOV = viewInfo.DesiredFOV;

	// 나머지 view 초기화 옵션을 작성
	ViewInitOptions.ViewFamily = ViewFamily;

	APlayerController* PlayerController = LocalPlayer->PlayerController;

	if (!PlayerController->bRenderPrimitiveComponents)
	{
		// 비어있는 화면 배치.
		ViewInitOptions.ShowOnlyPrimitives.Emplace();
	}
	else
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_BuildHiddenComponentList);
		PlayerController->BuildHiddenComponentList(OutViewLocation, /*out*/ ViewInitOptions.HiddenPrimitives);
	}

	//@TODO: SPLITSCREEN: show flags가 뷰 패밀리 전체에서 공유되므로, 이 호출에는 화면 분할 문제가 발생할 수 있습니다.
	EngineShowFlagOrthographicOverride(ViewInitOptions.IsPerspectiveProjection(), ViewFamily->EngineShowFlags);

	FSceneView* const view = new FSceneView(ViewInitOptions);

	view->ViewLocation = OutViewLocation;
	view->ViewRotation = OutViewRotation;
	// 뷰 정보서 이전 뷰 변환을 전달 합니다 (설정하면, 카메라에서 사용할수 있음).
	view->PreviousViewTransform = viewInfo.PreviousViewTransform;

	ViewFamily->Views.Add(view);

	{
		view->StartFinalPostprocessSettings(OutViewLocation);

		// CameraAnim override
		if (PlayerController->PlayerCameraManager)
		{
			TArray<FPostProcessSettings> const* CameraAnimPPSettings;
			TArray<float> const* CameraAnimPPBlendWeights;
			PlayerController->PlayerCameraManager->GetCachedPostProcessBlends(CameraAnimPPSettings, CameraAnimPPBlendWeights);

			for (int32 PPIdx = 0; PPIdx < CameraAnimPPBlendWeights->Num(); ++PPIdx)
			{
				view->OverridePostProcessSettings((*CameraAnimPPSettings)[PPIdx], (*CameraAnimPPBlendWeights)[PPIdx]);
			}
		}

		//	CAMERA OVERRIDE
		//	NOTE: Matinee 는 이 채널을 통해 작동합니다.
		view->OverridePostProcessSettings(viewInfo.PostProcessSettings, viewInfo.PostProcessBlendWeight);

		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->UpdatePhotographyPostProcessing(view->FinalPostProcessSettings);
		}

		if (GEngine->StereoRenderingDevice.IsValid())
		{
			FPostProcessSettings StereoDeviceOverridePostProcessinSettings;
			float BlendWeight = 1.0f;
			bool StereoSettingsAvailable = GEngine->StereoRenderingDevice->OverrideFinalPostprocessSettings(&StereoDeviceOverridePostProcessinSettings, view->StereoPass, view->StereoViewIndex, BlendWeight);
			if (StereoSettingsAvailable)
			{
				view->OverridePostProcessSettings(StereoDeviceOverridePostProcessinSettings, BlendWeight);
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		ADebugCameraController* debugCameraController = Cast<ADebugCameraController>(PlayerController);
		if (debugCameraController != nullptr)
		{
			debugCameraController->UpdateVisualizeBufferPostProcessing(view->FinalPostProcessSettings);
		}
#endif

		view->EndFinalPostprocessSettings(ViewInitOptions);
	}

	for (int ViewExt = 0; ViewExt < ViewFamily->ViewExtensions.Num(); ViewExt++)
	{
		ViewFamily->ViewExtensions[ViewExt]->SetupView(*ViewFamily, *view);
	}

	LocalPlayer->Origin = originCache;
	LocalPlayer->Size = sizeCache;

	return view;
}

void UNxCodeWindowsGameViewportClient::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	// 유효한 SceneCanvas가 필요합니다.
	check(SceneCanvas);

	UWorld* myWorld = GetWorld();
	for (FLocalPlayerIterator Iterator(GEngine, myWorld); Iterator; ++Iterator)
	{
		ULocalPlayer* localPlayer = *Iterator;
		if (localPlayer)
		{
			UNxCodeWindowsLocalPlayer* multiWindowsLocalPlayer = Cast<UNxCodeWindowsLocalPlayer>(localPlayer);
			if (multiWindowsLocalPlayer)
			{
				if (Window)
				{
					multiWindowsLocalPlayer->ViewManager	  = Window->ViewManager;
					multiWindowsLocalPlayer->EnableMultiViews = Window->ViewManager.EnableMultiViews;
				}
				else
				{
					multiWindowsLocalPlayer->EnableMultiViews = false;
				}
			}
		}
	}

	Super::Draw(InViewport, SceneCanvas);

	// Draw() 후에 NxCodeWindowsLocalPlayer 상태로 복원합니다.	
	for (FLocalPlayerIterator Iterator(GEngine, myWorld); Iterator; ++Iterator)
	{
		ULocalPlayer* localPlayer = *Iterator;
		if (localPlayer)
		{
			UNxCodeWindowsLocalPlayer* multiWindowsLocalPlayer = Cast<UNxCodeWindowsLocalPlayer>(localPlayer);
			if (multiWindowsLocalPlayer)
			{
				multiWindowsLocalPlayer->ViewManager = FNxCodeViewManager();
				multiWindowsLocalPlayer->EnableMultiViews = false;
			}
		}
	}
	 
	return;

	// 테스트 Draw ------------------------------------------------------------------------------------------------------ 코드 수정해야 함.
#if 0	
	{
		OnBeginDraw().Broadcast();

		bool hasValidViewsInWindow = Window && ((Window->ViewManager.Views.Num() > 0 && Window->ViewManager.EnableMultiViews) || Window->DoNotShowAnyView);
		// MultiView 모드에 대해 StereoRendering을 비활성화합니다.
		//	- 사용자는 MultiViews 모드를 사용하여 StereoRendering 효과를 구성할 수도 있기 때문입니다.
		const bool bStereoRendering = hasValidViewsInWindow ? false : GEngine->IsStereoscopic3D(InViewport);

		FCanvas* debugCanvas = InViewport->GetDebugCanvas();

		// 아직 임시 캔버스가 없다면 새로 만듭니다.
		static FName canvasObjectName(TEXT("CanvasObject"));
		UCanvas* canvasObject = GetCanvasByName(canvasObjectName);
		canvasObject->Canvas = SceneCanvas;

		// 임시 디버그 캔버스 개체 만들기
		FIntPoint debugCanvasSize = InViewport->GetSizeXY();
		if (bStereoRendering && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice())
		{
			debugCanvasSize = GEngine->XRSystem->GetHMDDevice()->GetIdealDebugCanvasRenderTargetSize();
		}

		static FName debugCanvasObjectName(TEXT("DebugCanvasObject"));
		UCanvas* debugCanvasObject = GetCanvasByName(debugCanvasObjectName);
		debugCanvasObject->Init(debugCanvasSize.X, debugCanvasSize.Y, NULL, debugCanvas);

		if (debugCanvas)
		{
			debugCanvas->SetScaledToRenderTarget(bStereoRendering);
			debugCanvas->SetStereoRendering(bStereoRendering);
		}
		if (SceneCanvas)
		{
			SceneCanvas->SetScaledToRenderTarget(bStereoRendering);
			SceneCanvas->SetStereoRendering(bStereoRendering);
		}

		UWorld* myWorld = GetWorld();

		// path tracing view mode 강제 설정, Flag 를 외부로 노출
		const bool bForcePathTracing = InViewport->GetClient()->GetEngineShowFlags()->PathTracing;
		if (bForcePathTracing)
		{
			EngineShowFlags.SetPathTracing(true);
			ViewModeIndex = VMI_PathTracing;
		}

		// 뷰포트의 렌더링 대상에 World 장면을 렌더링하기 위한 뷰 패밀리를 만듭니다.
		FSceneViewFamilyContext viewFamily(FSceneViewFamily::ConstructionValues(
			InViewport,
			myWorld->Scene,
			EngineShowFlags)
			.SetRealtimeUpdate(true));

	#if WITH_EDITOR
		if (GIsEditor)
		{
			// HighDPI 용 screen percentage 에 대한 뷰 패밀리 표시 플래그를 강제로 활성화합니다
			viewFamily.EngineShowFlags.ScreenPercentage = true;
		}

		UpdateDebugViewModeShaders();
	#endif

		viewFamily.ViewExtensions = GEngine->ViewExtensions->GatherActiveExtensions(InViewport);

		for (auto viewExt : viewFamily.ViewExtensions)
		{
			viewExt->SetupViewFamily(viewFamily);
		}

		if (bStereoRendering && GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice())
		{
			// HMD가 화면 설정을 수정하도록 허용
			GEngine->XRSystem->GetHMDDevice()->UpdateScreenSettings(Viewport);
		}

		ESplitScreenType::Type splitScreenConfig = GetCurrentSplitscreenConfiguration();
		viewFamily.ViewMode = EViewModeIndex(ViewModeIndex);
		EngineShowFlagOverride(ESFIM_Game, viewFamily.ViewMode, viewFamily.EngineShowFlags, false);

		if (viewFamily.EngineShowFlags.VisualizeBuffer && AllowDebugViewmodes())
		{
			// 버퍼 시각화 콘솔 명령 처리
			FName newBufferVisualizationMode = NAME_None;
			static IConsoleVariable* ICVar = IConsoleManager::Get().FindConsoleVariable(FBufferVisualizationData::GetVisualizationTargetConsoleCommandName());
			if (ICVar)
			{
				static const FName overviewName = TEXT("Overview");
				FString modeNameString = ICVar->GetString();
				FName modeName = *modeNameString;
				if (modeNameString.IsEmpty() || modeName == overviewName || modeName == NAME_None)
				{
					newBufferVisualizationMode = NAME_None;
				}
				else
				{
					if (GetBufferVisualizationData().GetMaterial(modeName) == NULL)
					{
						// 모드가 범위를 벗어났으므로 사용자에게 메시지를 표시하고 이전 유효 모드로 다시 설정합니다.
						UE_LOG(LogConsoleResponse, Warning, TEXT("Buffer visualization mode '%s' does not exist"), *modeNameString);
						newBufferVisualizationMode = GetCurrentBufferVisualizationMode();
						// todo: cvars는 사용자 설정입니다. 
						//	- cvar 상태는 로그 스팸을 방지하고 사용자를 자동으로 수정하는 데 사용됩니다(사용자가 원하는 것이 아닐 가능성이 높음).
						ICVar->Set(*newBufferVisualizationMode.GetPlainNameString(), ECVF_SetByCode);
					}
					else
					{
						newBufferVisualizationMode = modeName;
					}
				}
			}

			if (newBufferVisualizationMode != GetCurrentBufferVisualizationMode())
			{
				SetCurrentBufferVisualizationMode(newBufferVisualizationMode);
			}
		}

		TMap<ULocalPlayer*, FSceneView*> playerViewMap;

		FAudioDevice* audioDevice = myWorld->GetAudioDeviceRaw();
		TArray<FSceneView*> views;

		for (FLocalPlayerIterator Iterator(GEngine, myWorld); Iterator; ++Iterator)
		{
			ULocalPlayer* localPlayer = *Iterator;
			if (localPlayer)
			{
				APlayerController* playerController = localPlayer->PlayerController;

				int32 numViewsInWindow = Window ? Window->ViewManager.Views.Num() : 1;
				if (numViewsInWindow < 1)
				{
					numViewsInWindow = 1;
				}
				else if(numViewsInWindow > UVxMultiWindowsGameViewportClient::MaxNumOfViews)
				{
					numViewsInWindow = UVxMultiWindowsGameViewportClient::MaxNumOfViews;
				}
				if (Window && Window->DoNotShowAnyView)
				{
					numViewsInWindow = 0;
				}

				const int32 NumViews = bStereoRendering ? GEngine->StereoRenderingDevice->GetDesiredNumberOfViews(bStereoRendering) : numViewsInWindow;

				for (int32 i = 0; i < NumViews; ++i)
				{
					// 플레이어의 뷰 정보를 계산합니다.
					FVector		viewLocation;
					FRotator	viewRotation;
					EStereoscopicPass passType = bStereoRendering ? GEngine->StereoRenderingDevice->GetViewPassForIndex(bStereoRendering, i) : EStereoscopicPass::eSSP_FULL;

					FSceneView* view = nullptr;
					if (hasValidViewsInWindow)
					{
						view = CalcSceneView(localPlayer, &viewFamily, viewLocation, viewRotation, InViewport, i, nullptr, (int32)passType);
					}
					else
					{
						view = localPlayer->CalcSceneView(&viewFamily, viewLocation, viewRotation, InViewport, nullptr, (int32)passType);
					}
				
					if (view)
					{
						views.Add(view);

						if (view->Family->EngineShowFlags.Wireframe)
						{
							// 와이어프레임 색상은 emissive-only 이며 메시 수정 재질은 재질 대체를 사용하지 않으므로...
							view->DiffuseOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
							view->SpecularOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
						}
						else if (view->Family->EngineShowFlags.OverrideDiffuseAndSpecular)
						{
							view->DiffuseOverrideParameter  = FVector4f(GEngine->LightingOnlyBrightness.R, GEngine->LightingOnlyBrightness.G, GEngine->LightingOnlyBrightness.B, 0.0f);
							view->SpecularOverrideParameter = FVector4f(.1f, .1f, .1f, 0.0f);
						}
						else if (view->Family->EngineShowFlags.ReflectionOverride)
						{
							view->DiffuseOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
							view->SpecularOverrideParameter = FVector4f(1, 1, 1, 0.0f);
							view->NormalOverrideParameter = FVector4f(0, 0, 1, 0.0f);
							view->RoughnessOverrideParameter = FVector2D(0.0f, 0.0f);
						}

						if (!view->Family->EngineShowFlags.Diffuse)
						{
							view->DiffuseOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
						}

						if (!view->Family->EngineShowFlags.Specular)
						{
							view->SpecularOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
						}

						view->CurrentBufferVisualizationMode = GetCurrentBufferVisualizationMode();

						view->CameraConstrainedViewRect = view->UnscaledViewRect;

						// 이것이 기본 드로잉 패스인 경우 뷰 위치에 따라 업데이트됩니다.
						if (i == 0)
						{
							// 뷰의 위치를 저장합니다.
							localPlayer->LastViewLocation = viewLocation;

							playerViewMap.Add(localPlayer, view);

							// 리스너를 업데이트
							if (audioDevice != NULL && playerController != NULL)
							{
								bool bUpdateListenerPosition = true;
																
								// 주 오디오 장치가 여러 PIE 뷰포트 클라이언트에 사용되는 경우,
								// 초점이 맞춰진 경우에 만, 주 오디오 장치 리스너 위치를 업데이트 합니다.
								if (GEngine)
								{
									FAudioDeviceManager* audioDeviceManager = GEngine->GetAudioDeviceManager();

									// 주 오디오 장치를 참조하는 월드가 둘 이상인 경우
									if (audioDeviceManager->GetNumMainAudioDeviceWorlds() > 1)
									{
										FAudioDeviceHandle mainAudioDeviceHandle = GEngine->GetAudioDeviceHandle();
										if (audioDevice->GetMainAudioDevice() == mainAudioDeviceHandle && !HasAudioFocus())
										{
											bUpdateListenerPosition = false;
										}
									}
								}

								if (bUpdateListenerPosition)
								{
									FVector location;
									FVector projFront;
									FVector projRight;
									playerController->GetAudioListenerPosition(/*out*/ location, /*out*/ projFront, /*out*/ projRight);

									FTransform listenerTransform(FRotationMatrix::MakeFromXY(projFront, projRight));

									// HMD가 뷰 위치가 아닌 플레이어의 머리 위치에 따라 조정되도록 허용
									if (GEngine->XRSystem.IsValid() && GEngine->StereoRenderingDevice.IsValid() && GEngine->StereoRenderingDevice->IsStereoEnabled())
									{
										const FVector Offset = GEngine->XRSystem->GetAudioListenerOffset();
										location += listenerTransform.TransformPositionNoScale(Offset);
									}

									listenerTransform.SetTranslation(location);
									listenerTransform.NormalizeRotation();

									uint32 viewportIndex = playerViewMap.Num() - 1;
									audioDevice->SetListener(myWorld, viewportIndex, listenerTransform, (view->bCameraCut ? 0.f : myWorld->GetDeltaSeconds()));

									FVector overrideAttenuation;
									if (playerController->GetAudioListenerAttenuationOverridePosition(overrideAttenuation))
									{
										audioDevice->SetListenerAttenuationOverride(overrideAttenuation);
									}
									else
									{
										audioDevice->ClearListenerAttenuationOverride();
									}
								}
							}

						#if RHI_RAYTRACING
							// view->bAllowRayTracing = true;
						#endif

						#if CSV_PROFILER
							UpdateCsvCameraStats(view);
						#endif
						}

						// 리소스 스트리밍에 대한 보기 정보를 추가합니다. 작은 FOV의 경우 최대 5배 부스트를 허용합니다.
						const float streamingScale = 1.f / FMath::Clamp<float>(view->LODDistanceFactor, .2f, 1.f);
						IStreamingManager::Get().AddViewInformation(view->ViewMatrices.GetViewOrigin(), view->UnscaledViewRect.Width(), view->UnscaledViewRect.Width() * view->ViewMatrices.GetProjectionMatrix().M[0][0], streamingScale);
						myWorld->ViewLocationsRenderedLastFrame.Add(view->ViewMatrices.GetViewOrigin());
					}
				}
			}
		}

		FinalizeViews(&viewFamily, playerViewMap);

		// 레벨 스트리밍을 업데이트합니다.
		myWorld->UpdateLevelStreaming();

		// 렌더링된 모든 뷰로 둘러싸인 가장 큰 직사각형을 찾습니다.
		uint32 MinX = InViewport->GetSizeXY().X, MinY = InViewport->GetSizeXY().Y, MaxX = 0, MaxY = 0;
		uint32 TotalArea = 0;
		{
			for (int32 ViewIndex = 0; ViewIndex < viewFamily.Views.Num(); ++ViewIndex)
			{
				const FSceneView* View = viewFamily.Views[ViewIndex];

				FIntRect UpscaledViewRect = View->UnscaledViewRect;

				MinX = FMath::Min<uint32>(UpscaledViewRect.Min.X, MinX);
				MinY = FMath::Min<uint32>(UpscaledViewRect.Min.Y, MinY);
				MaxX = FMath::Max<uint32>(UpscaledViewRect.Max.X, MaxX);
				MaxY = FMath::Max<uint32>(UpscaledViewRect.Max.Y, MaxY);
				TotalArea += FMath::TruncToInt(UpscaledViewRect.Width()) * FMath::TruncToInt(UpscaledViewRect.Height());
			}

			// 렌더링된 이미지 주위에 검은색 테두리를 그리기 (이미지 외부를 읽는 후처리 패스(예: PostProcessAA)로 인한 아티팩트 방지)
			{
				int32 BlackBorders = FMath::Clamp(CVarSetBlackBordersEnabled.GetValueOnGameThread(), 0, 10);

				if (viewFamily.Views.Num() == 1 && BlackBorders)
				{
					MinX += BlackBorders;
					MinY += BlackBorders;
					MaxX -= BlackBorders;
					MaxY -= BlackBorders;
					TotalArea = (MaxX - MinX) * (MaxY - MinY);
				}
			}
		}

		// 뷰가 전체 경계 사각형을 덮지 않으면 전체 버퍼를 지웁니다.
		bool bBufferCleared = false;
		bool bStereoscopicPass = (viewFamily.Views.Num() != 0 && viewFamily.Views[0]->StereoPass != EStereoscopicPass::eSSP_FULL);
		if (viewFamily.Views.Num() == 0 || TotalArea != (MaxX - MinX) * (MaxY - MinY) || bDisableWorldRendering || bStereoscopicPass)
		{
			// 입체 뷰(stereoscopic views)일 때는 TotalArea 계산이 올바르게 처리되지 않는다.
			if (bDisableWorldRendering || !bStereoscopicPass) 
			{
				SceneCanvas->Clear(FLinearColor::Transparent);
			}

			bBufferCleared = true;
		}

		// 지원되지 않는 경우 화면 비율 표시 플래그를 강제로 해제합니다.
		if (!viewFamily.SupportsScreenPercentage())
		{
			viewFamily.EngineShowFlags.ScreenPercentage = false;
		}

		// Set up secondary resolution fraction for the view family.
		if (!bStereoRendering && viewFamily.SupportsScreenPercentage())
		{
			float CustomSecondaruScreenPercentage = CVarSecondaryScreenPercentage.GetValueOnGameThread();

			if (CustomSecondaruScreenPercentage > 0.0)
			{
				// Override secondary resolution fraction with CVar.
				viewFamily.SecondaryViewFraction = FMath::Min(CustomSecondaruScreenPercentage / 100.0f, 1.0f);
			}
			else
			{
				// Automatically compute secondary resolution fraction from DPI.
				viewFamily.SecondaryViewFraction = GetDPIDerivedResolutionFraction();
			}

			check(viewFamily.SecondaryViewFraction > 0.0f);
		}

		checkf(viewFamily.GetScreenPercentageInterface() == nullptr,
			TEXT("Some code has tried to set up an alien screen percentage driver, that could be wrong if not supported very well by the RHI."));

		// Setup main view family with screen percentage interface by dynamic resolution if screen percentage is enabled.
	#if WITH_DYNAMIC_RESOLUTION
		if (viewFamily.EngineShowFlags.ScreenPercentage)
		{
			FDynamicResolutionStateInfos dynamicResStateInfos;
			GEngine->GetDynamicResolutionCurrentStateInfos(/* out */ dynamicResStateInfos);

			// Do not allow dynamic resolution to touch the view family if not supported to ensure there is no possibility to ruin
			// game play experience on platforms that does not support it, but have it enabled by mistake.
			if (dynamicResStateInfos.Status == EDynamicResolutionStatus::Enabled)
			{
				GEngine->EmitDynamicResolutionEvent(EDynamicResolutionStateEvent::BeginDynamicResolutionRendering);
				GEngine->GetDynamicResolutionState()->SetupMainViewFamily(viewFamily);
			}
			else if (dynamicResStateInfos.Status == EDynamicResolutionStatus::DebugForceEnabled)
			{
				GEngine->EmitDynamicResolutionEvent(EDynamicResolutionStateEvent::BeginDynamicResolutionRendering);
				viewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
					viewFamily,
					dynamicResStateInfos.ResolutionFractionApproximation,
					/* AllowPostProcessSettingsScreenPercentage = */ false,
					dynamicResStateInfos.ResolutionFractionUpperBound));
			}

	#if CSV_PROFILER
			if (dynamicResStateInfos.ResolutionFractionApproximation >= 0.0f)
			{
				CSV_CUSTOM_STAT_GLOBAL(DynamicResolutionPercentage, dynamicResStateInfos.ResolutionFractionApproximation * 100.0f, ECsvCustomStatOp::Set);
			}
	#endif
		}
	#endif

		// If a screen percentage interface was not set by dynamic resolution, then create one matching legacy behavior.
		if (viewFamily.GetScreenPercentageInterface() == nullptr)
		{
			bool AllowPostProcessSettingsScreenPercentage = false;
			float GlobalResolutionFraction = 1.0f;

			if (viewFamily.EngineShowFlags.ScreenPercentage)
			{
				// Allow FPostProcessSettings::ScreenPercentage.
				AllowPostProcessSettingsScreenPercentage = true;

				// Get global view fraction set by r.ScreenPercentage.
				GlobalResolutionFraction = FLegacyScreenPercentageDriver::GetCVarResolutionFraction();
			}

			viewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
				viewFamily, GlobalResolutionFraction, AllowPostProcessSettingsScreenPercentage));
		}
		else if (bStereoRendering)
		{
			// Change screen percentage method to raw output when doing dynamic resolution with VR if not using TAA upsample.
			for (FSceneView* View : views)
			{
				if (View->PrimaryScreenPercentageMethod == EPrimaryScreenPercentageMethod::SpatialUpscale)
				{
					View->PrimaryScreenPercentageMethod = EPrimaryScreenPercentageMethod::RawOutput;
				}
			}
		}

		// Draw the player views.
		if (!bDisableWorldRendering && playerViewMap.Num() > 0 && FSlateApplication::Get().GetPlatformApplication()->IsAllowedToRender()) //-V560
		{
			GetRendererModule().BeginRenderingViewFamily(SceneCanvas, &viewFamily);
		}
		else
		{
			// Make sure RHI resources get flushed if we're not using a renderer
			ENQUEUE_RENDER_COMMAND(UGameViewportClient_FlushRHIResources)(
				[](FRHICommandListImmediate& RHICmdList)
				{
					RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
				});
		}

		// Beyond this point, only UI rendering independent from dynamc resolution.
		GEngine->EmitDynamicResolutionEvent(EDynamicResolutionStateEvent::EndDynamicResolutionRendering);

		// Clear areas of the rendertarget (backbuffer) that aren't drawn over by the views.
		if (!bBufferCleared)
		{
			// clear left
			if (MinX > 0)
			{
				SceneCanvas->DrawTile(0, 0, MinX, InViewport->GetSizeXY().Y, 0.0f, 0.0f, 1.0f, 1.f, FLinearColor::Black, NULL, false);
			}
			// clear right
			if (MaxX < (uint32)InViewport->GetSizeXY().X)
			{
				SceneCanvas->DrawTile(MaxX, 0, InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0.0f, 0.0f, 1.0f, 1.f, FLinearColor::Black, NULL, false);
			}
			// clear top
			if (MinY > 0)
			{
				SceneCanvas->DrawTile(MinX, 0, MaxX, MinY, 0.0f, 0.0f, 1.0f, 1.f, FLinearColor::Black, NULL, false);
			}
			// clear bottom
			if (MaxY < (uint32)InViewport->GetSizeXY().Y)
			{
				SceneCanvas->DrawTile(MinX, MaxY, MaxX, InViewport->GetSizeXY().Y, 0.0f, 0.0f, 1.0f, 1.f, FLinearColor::Black, NULL, false);
			}
		}

		// Remove temporary debug lines.
		if (myWorld->LineBatcher != nullptr)
		{
			myWorld->LineBatcher->Flush();
		}

		if (myWorld->ForegroundLineBatcher != nullptr)
		{
			myWorld->ForegroundLineBatcher->Flush();
		}

		// Draw FX debug information.
		if (myWorld->FXSystem)
		{
			myWorld->FXSystem->DrawDebug(SceneCanvas);
		}

		// Render the UI.
		if (FSlateApplication::Get().GetPlatformApplication()->IsAllowedToRender())
		{
			SCOPE_CYCLE_COUNTER(STAT_UIDrawingTime_MultiWindowsGameViewportClient);
			CSV_SCOPED_TIMING_STAT_EXCLUSIVE(UI);

			// render HUD
			bool bDisplayedSubtitles = false;
			for (FConstPlayerControllerIterator Iterator = myWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				APlayerController* PlayerController = Iterator->Get();
				if (PlayerController)
				{
					ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);
					if (LocalPlayer)
					{
						FSceneView* View = playerViewMap.FindRef(LocalPlayer);
						if (View != NULL)
						{
							// 뷰포트 대상으로 직접 렌더링
							FVector CanvasOrigin(FMath::TruncToFloat(View->UnscaledViewRect.Min.X), FMath::TruncToInt(View->UnscaledViewRect.Min.Y), 0.f);

							canvasObject->Init(View->UnscaledViewRect.Width(), View->UnscaledViewRect.Height(), View, SceneCanvas);

							// 플레이어의 viewRect에 대한 캔버스 변환을 설정합니다.
							check(SceneCanvas);
							SceneCanvas->PushAbsoluteTransform(FTranslationMatrix(CanvasOrigin));
							canvasObject->ApplySafeZoneTransform();

							// Render the player's HUD.
							if (PlayerController->MyHUD)
							{
								SCOPE_CYCLE_COUNTER(STAT_HudTime_MultiWindowsGameViewportClient);

								debugCanvasObject->SceneView = View;
								PlayerController->MyHUD->SetCanvas(canvasObject, debugCanvasObject);

								PlayerController->MyHUD->PostRender();

								// Put these pointers back as if a blueprint breakpoint hits during HUD PostRender they can
								// have been changed
								canvasObject->Canvas = SceneCanvas;
								debugCanvasObject->Canvas = debugCanvas;

								// A side effect of PostRender is that the playercontroller could be destroyed
								if (!PlayerController->IsPendingKill())
								{
									PlayerController->MyHUD->SetCanvas(NULL, NULL);
								}
							}

							if (debugCanvas != NULL)
							{
								debugCanvas->PushAbsoluteTransform(FTranslationMatrix(CanvasOrigin));
								UDebugDrawService::Draw(viewFamily.EngineShowFlags, InViewport, View, debugCanvas, debugCanvasObject);
								debugCanvas->PopTransform();
							}

							canvasObject->PopSafeZoneTransform();
							SceneCanvas->PopTransform();

							// draw subtitles
							if (!bDisplayedSubtitles)
							{
								FVector2D MinPos(0.f, 0.f);
								FVector2D MaxPos(1.f, 1.f);
								GetSubtitleRegion(MinPos, MaxPos);

								const uint32 SizeX = SceneCanvas->GetRenderTarget()->GetSizeXY().X;
								const uint32 SizeY = SceneCanvas->GetRenderTarget()->GetSizeXY().Y;
								FIntRect SubtitleRegion(FMath::TruncToInt(SizeX * MinPos.X), FMath::TruncToInt(SizeY * MinPos.Y), FMath::TruncToInt(SizeX * MaxPos.X), FMath::TruncToInt(SizeY * MaxPos.Y));
								FSubtitleManager::GetSubtitleManager()->DisplaySubtitles(SceneCanvas, SubtitleRegion, myWorld->GetAudioTimeSeconds());
								bDisplayedSubtitles = true;
							}
						}
					}
				}
			}

			//ensure canvas has been flushed before rendering UI
			SceneCanvas->Flush_GameThread();

			OnDrawn().Broadcast();

			// Allow the viewport to render additional stuff
			PostRender(debugCanvasObject);
		}


		// Grab the player camera location and orientation so we can pass that along to the stats drawing code.
		FVector PlayerCameraLocation = FVector::ZeroVector;
		FRotator PlayerCameraRotation = FRotator::ZeroRotator;
		{
			for (FConstPlayerControllerIterator Iterator = myWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				if (APlayerController* PC = Iterator->Get())
				{
					PC->GetPlayerViewPoint(PlayerCameraLocation, PlayerCameraRotation);
				}
			}
		}

		if (debugCanvas)
		{
			// Reset the debug canvas to be full-screen before drawing the console
			// (the debug draw service above has messed with the viewport size to fit it to a single player's subregion)
			debugCanvasObject->Init(debugCanvasSize.X, debugCanvasSize.Y, NULL, debugCanvas);

			DrawStatsHUD(myWorld, InViewport, debugCanvas, debugCanvasObject, DebugProperties, PlayerCameraLocation, PlayerCameraRotation);

			// if (GEngine->IsStereoscopic3D(InViewport))
			if(bStereoRendering)
			{
	#if 0 //!UE_BUILD_SHIPPING
				// TODO: replace implementation in OculusHMD with a debug renderer
				if (GEngine->XRSystem.IsValid())
				{
					GEngine->XRSystem->DrawDebug(DebugCanvasObject);
				}
	#endif
			}

			// Render the console absolutely last because developer input is was matter the most.
			if (ViewportConsole)
			{
				ViewportConsole->PostRender_Console(debugCanvasObject);
			}
		}

		OnEndDraw().Broadcast();
	}
#endif

}


#undef LOCTEXT_NAMESPACE
