
#include "NxCodeWindowsLocalPlayer.h"

#include "Engine/LocalPlayer.h"
#include "Misc/FileHelper.h"
#include "EngineDefines.h"
#include "EngineGlobals.h"
#include "Engine/Scene.h"
#include "Camera/CameraTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "SceneView.h"
#include "UObject/UObjectAnnotation.h"
#include "Logging/LogScopedCategoryAndVerbosityOverride.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/PlayerController.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "UnrealEngine.h"
#include "EngineUtils.h"

#include "Net/OnlineEngineInterface.h"
#include "SceneManagement.h"
#include "PhysicsPublic.h"
#include "SkeletalMeshTypes.h"
#include "HAL/PlatformApplicationMisc.h"

#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "IXRCamera.h"
#include "SceneViewExtension.h"
#include "Net/DataChannel.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "NxCodeWindowsLocalPlayer"

DEFINE_LOG_CATEGORY(LogCodeWindowsLocalPlayer);

DECLARE_CYCLE_STAT(TEXT("CalcView_Custom"), STAT_CalcView_Custom, STATGROUP_Engine);

const int32 UNxCodeWindowsLocalPlayer::MaxNumOfViews =20;

UNxCodeWindowsLocalPlayer::UNxCodeWindowsLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UNxCodeWindowsLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	//FView view;
	//view.ViewpointType = EViewPointType::BindToPlayerController;
	//view.LocationAndSizeOnScreen = FVector4(0.0f, 0.0f, 1.0f, 1.0f);
	//ViewManager.Views.Add(view);

	ViewMode = E_ViewModeType::VMI_Lit;

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

void UNxCodeWindowsLocalPlayer::BeginDestroy()
{
	Super::BeginDestroy();
}

void UNxCodeWindowsLocalPlayer::FinishDestroy()
{
	if (true)
	{
		for (FSceneViewStateReference& ViewStateTemp : ViewStates)
		{
			ViewStateTemp.Destroy();
		}
	}
	Super::FinishDestroy();
}

void UNxCodeWindowsLocalPlayer::GetViewPoint(FMinimalViewInfo& OutViewInfo) const
{
	Super::GetViewPoint(OutViewInfo);

	if (EnableMultiViews)
	{
		if (ViewManager.Views.Num() > CurrentViewIndex)
		{
			FNxCodeView currentView = ViewManager.Views[CurrentViewIndex];
			if (currentView.ViewpointType == E_ViewPointType::CustomViewPoint)
			{
				currentView.CustomViewPoint.CustomPOV.CopyToViewInfo(OutViewInfo);
			}
			else if (currentView.ViewpointType == E_ViewPointType::BindToViewTarget)
			{
				currentView.BindToViewTarget.ApplyToViewInfo(OutViewInfo);
			}
		}

		OffsetViewLocationAndRotation(OutViewInfo);
	}
}

void UNxCodeWindowsLocalPlayer::OffsetViewLocationAndRotation(FMinimalViewInfo& InOutViewInfo) const
{
	if (EnableMultiViews)
	{
		if (ViewManager.Views.Num() > CurrentViewIndex)
		{
			FNxCodeView currentView = ViewManager.Views[CurrentViewIndex];
			
			FVector  viewLocation = InOutViewInfo.Location;
			FRotator viewRotation = InOutViewInfo.Rotation;

			const FTransform viewRelativeTransform(currentView.RotationOffsetOfViewpoint, currentView.LocationOffsetOfViewpoint);
			const FTransform viewWorldTransform(viewRotation, viewLocation);
			FTransform newViewWorldTransform = viewRelativeTransform*viewWorldTransform;

			viewLocation = newViewWorldTransform.GetLocation();
			viewRotation = newViewWorldTransform.GetRotation().Rotator();

			InOutViewInfo.Location = viewLocation;
			InOutViewInfo.Rotation = viewRotation;
		}
	}
}

bool UNxCodeWindowsLocalPlayer::CalcSceneViewInitOptions_Custom(
	struct FSceneViewInitOptions& ViewInitOptions,
	FViewport* Viewport,
	const int32 IndexOfView,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	// CalcSceneViewInitOptions(ViewInitOptions, Viewport, ViewDrawer, StereoPass);

	if ((PlayerController == NULL) || (Size.X <= 0.f) || (Size.Y <= 0.f) || (Viewport == NULL))
	{
		return false;
	}

	// get the projection data
	if (GetProjectionData(Viewport, /*inout*/ ViewInitOptions, StereoViewIndex) == false)
	{
		// Return NULL if this we didn't get back the info we needed
		return false;
	}

	// return if we have an invalid view rect
	if (!ViewInitOptions.IsValidViewRectangle())
	{
		return false;
	}

	if (PlayerController->PlayerCameraManager != NULL)
	{
		// 화면 페이드 효과를 화면에 적용
		if (PlayerController->PlayerCameraManager->bEnableFading)
		{
			ViewInitOptions.OverlayColor = PlayerController->PlayerCameraManager->FadeColor;
			ViewInitOptions.OverlayColor.A = FMath::Clamp(PlayerController->PlayerCameraManager->FadeAmount, 0.0f, 1.0f);
		}

		// 원하는 경우 색상 스케일링을 수행
		if (PlayerController->PlayerCameraManager->bEnableColorScaling)
		{
			ViewInitOptions.ColorScale = FLinearColor(
				PlayerController->PlayerCameraManager->ColorScale.X,
				PlayerController->PlayerCameraManager->ColorScale.Y,
				PlayerController->PlayerCameraManager->ColorScale.Z
			);
		}

		// 이 프레임에 카메라 컷인가?
		ViewInitOptions.bInCameraCut = PlayerController->PlayerCameraManager->bGameCameraCutThisFrame;
	}

	if (GEngine->StereoRenderingDevice.IsValid())
	{
		ViewInitOptions.StereoPass = GEngine->StereoRenderingDevice->GetViewPassForIndex(StereoViewIndex != INDEX_NONE, StereoViewIndex);
	}

	check(PlayerController && PlayerController->GetWorld());

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

	ViewInitOptions.ViewActor = PlayerController->GetViewTarget();
	ViewInitOptions.PlayerIndex = GetControllerId();
	ViewInitOptions.ViewElementDrawer = ViewDrawer;
	ViewInitOptions.BackgroundColor = FLinearColor::Black;
	ViewInitOptions.LODDistanceFactor = PlayerController->LocalPlayerCachedLODDistanceFactor;
	ViewInitOptions.WorldToMetersScale = PlayerController->GetWorldSettings()->WorldToMeters;
	ViewInitOptions.CursorPos = Viewport->HasMouseCapture() ? FIntPoint(-1, -1) : FIntPoint(Viewport->GetMouseX(), Viewport->GetMouseY());
	ViewInitOptions.OriginOffsetThisFrame = PlayerController->GetWorld()->OriginOffsetThisFrame;

	return true;
}

FSceneView* UNxCodeWindowsLocalPlayer::CalcSceneView(class FSceneViewFamily* ViewFamily,
	FVector& OutViewLocation,
	FRotator& OutViewRotation,
	FViewport* Viewport,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	if (!EnableMultiViews)
	{
		return 	Super::CalcSceneView(ViewFamily,
			OutViewLocation,
			OutViewRotation,
			Viewport,
			ViewDrawer,
			StereoViewIndex);
	}
	else
	{
		return 	CalcMultiViews(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoViewIndex);
	}
}

FSceneView* UNxCodeWindowsLocalPlayer::CalcMultiViews(class FSceneViewFamily* ViewFamily,
	FVector& OutViewLocation,
	FRotator& OutViewRotation,
	FViewport* Viewport,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	FSceneView* View = nullptr;
	for (int32 IndexOfView = 0; IndexOfView < ViewManager.Views.Num() && IndexOfView < MaxNumOfViews; IndexOfView++)
	{
		CurrentViewIndex = IndexOfView;
		FNxCodeView currentView = ViewManager.Views[IndexOfView];
		FSceneView* viewTemp = CalcView_Custom(ViewFamily, OutViewLocation, OutViewRotation, Viewport, currentView, IndexOfView, ViewDrawer, StereoViewIndex);
		if (View == nullptr)
		{
			View = viewTemp;
		}
	}

	return 	View;
}

//static void SetupMonoParameters(FSceneViewFamily& ViewFamily, const FSceneView& MonoView)
//{
//	// Compute the NDC depths for the far field clip plane. This assumes symmetric projection.
//	const FMatrix& LeftEyeProjection = ViewFamily.Views[0]->ViewMatrices.GetProjectionMatrix();
//
//	// Start with a point on the far field clip plane in eye space. The mono view uses a point slightly biased towards the camera to ensure there's overlap.
//	const FVector4 StereoDepthCullingPointEyeSpace(0.0f, 0.0f, ViewFamily.MonoParameters.CullingDistance, 1.0f);
//	const FVector4 FarFieldDepthCullingPointEyeSpace(0.0f, 0.0f, ViewFamily.MonoParameters.CullingDistance - ViewFamily.MonoParameters.OverlapDistance, 1.0f);
//
//	// Project into clip space
//	const FVector4 ProjectedStereoDepthCullingPointClipSpace = LeftEyeProjection.TransformFVector4(StereoDepthCullingPointEyeSpace);
//	const FVector4 ProjectedFarFieldDepthCullingPointClipSpace = LeftEyeProjection.TransformFVector4(FarFieldDepthCullingPointEyeSpace);
//
//	// Perspective divide for NDC space
//	ViewFamily.MonoParameters.StereoDepthClip = ProjectedStereoDepthCullingPointClipSpace.Z / ProjectedStereoDepthCullingPointClipSpace.W;
//	ViewFamily.MonoParameters.MonoDepthClip = ProjectedFarFieldDepthCullingPointClipSpace.Z / ProjectedFarFieldDepthCullingPointClipSpace.W;
//
//	// We need to determine the stereo disparity difference between the center mono view and an offset stereo view so we can account for it when compositing.
//	// We take a point on a stereo view far field clip plane, unproject it, then reproject it using the mono view. The stereo disparity offset is then
//	// the difference between the original test point and the reprojected point.
//	const FVector4 ProjectedPointAtLimit(0.0f, 0.0f, ViewFamily.MonoParameters.MonoDepthClip, 1.0f);
//	const FVector4 WorldProjectedPoint = ViewFamily.Views[0]->ViewMatrices.GetInvViewProjectionMatrix().TransformFVector4(ProjectedPointAtLimit);
//	FVector4 MonoProjectedPoint = MonoView.ViewMatrices.GetViewProjectionMatrix().TransformFVector4(WorldProjectedPoint / WorldProjectedPoint.W);
//	MonoProjectedPoint = MonoProjectedPoint / MonoProjectedPoint.W;
//	ViewFamily.MonoParameters.LateralOffset = (MonoProjectedPoint.X - ProjectedPointAtLimit.X) / 2.0f;
//}


FSceneView* UNxCodeWindowsLocalPlayer::CalcView_Custom(class FSceneViewFamily* ViewFamily,
	FVector& OutViewLocation,
	FRotator& OutViewRotation,
	FViewport* Viewport,
	const FNxCodeView ViewSetting,
	const int32 IndexOfView,
	class FViewElementDrawer* ViewDrawer,
	int32 StereoViewIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_CalcView_Custom);

	FVector2D originCache = Origin;
	FVector2D sizeCache = Size;
	Origin = FVector2D(ViewSetting.LocationAndSizeOnScreen.X, ViewSetting.LocationAndSizeOnScreen.Y);
	Size   = FVector2D(ViewSetting.LocationAndSizeOnScreen.Z, ViewSetting.LocationAndSizeOnScreen.W);

	FSceneViewInitOptions ViewInitOptions;

	if (!CalcSceneViewInitOptions_Custom(ViewInitOptions, Viewport, IndexOfView, ViewDrawer, StereoViewIndex))
	{
		return nullptr;
	}

	// Get the viewpoint...technically doing this twice
	// but it makes GetProjectionData better
	FMinimalViewInfo viewInfo;
	GetViewPoint(viewInfo);
	OutViewLocation = viewInfo.Location;
	OutViewRotation = viewInfo.Rotation;
	ViewInitOptions.bUseFieldOfViewForLOD = viewInfo.bUseFieldOfViewForLOD;
	ViewInitOptions.FOV = viewInfo.FOV;
	ViewInitOptions.DesiredFOV = viewInfo.DesiredFOV;

	// Fill out the rest of the view init options
	ViewInitOptions.ViewFamily = ViewFamily;

	if (!PlayerController->bRenderPrimitiveComponents)
	{
		// Emplaces an empty show only primitive list.
		ViewInitOptions.ShowOnlyPrimitives.Emplace();
	}
	else
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_BuildHiddenComponentList);
		PlayerController->BuildHiddenComponentList(OutViewLocation, /*out*/ ViewInitOptions.HiddenPrimitives);
	}

	//@TODO: SPLITSCREEN: This call will have an issue with splitscreen, as the show flags are shared across the view family
	EngineShowFlagOrthographicOverride(ViewInitOptions.IsPerspectiveProjection(), ViewFamily->EngineShowFlags);

	FSceneView* const view = new FSceneView(ViewInitOptions);

	view->ViewLocation = OutViewLocation;
	view->ViewRotation = OutViewRotation;

	ViewFamily->Views.Add(view);

	{
		// Clear MIDPool of Views for MultiWindow. Otherwise the "Resource" property of MID maybe invalid and crash at "check(NewMID->GetRenderProxy());" in "ScenePrivate.h"
		if (view->State)
		{
			view->State->ClearMIDPool();
		}

		view->StartFinalPostprocessSettings(OutViewLocation);

		// CameraAnim override
		if (PlayerController != NULL && PlayerController->PlayerCameraManager)
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
		//	NOTE: Matinee works through this channel
		view->OverridePostProcessSettings(viewInfo.PostProcessSettings, viewInfo.PostProcessBlendWeight);

		view->EndFinalPostprocessSettings(ViewInitOptions);
	}

	for (int ViewExt = 0; ViewExt < ViewFamily->ViewExtensions.Num(); ViewExt++)
	{
		ViewFamily->ViewExtensions[ViewExt]->SetupView(*ViewFamily, *view);
	}

	// Monoscopic far field setup
	//if (ViewFamily->IsMonoscopicFarFieldEnabled() && StereoPass == eSSP_MONOSCOPIC_EYE)
	//{
	//	SetupMonoParameters(*ViewFamily, *View);
	//}

	Origin = originCache;
	Size = sizeCache;

	return view;
}

bool UNxCodeWindowsLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UNxCodeWindowsLocalPlayer::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return Super::Exec(InWorld, Cmd, Ar);
}

#undef LOCTEXT_NAMESPACE
