
#include "NxCodeView.h"
#include "Camera/CameraComponent.h"

void FNxViewInfo::CopyToViewInfo(FMinimalViewInfo& InOutInfo) const
{
	InOutInfo.Location = Location;
	InOutInfo.Rotation = Rotation;
	InOutInfo.FOV = FOV;
	InOutInfo.OrthoWidth = OrthoWidth;
	InOutInfo.OrthoNearClipPlane = OrthoNearClipPlane;
	InOutInfo.OrthoFarClipPlane = OrthoFarClipPlane;
	InOutInfo.AspectRatio = AspectRatio;
	InOutInfo.bConstrainAspectRatio = bConstrainAspectRatio;
	InOutInfo.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	InOutInfo.ProjectionMode = ProjectionMode;
	InOutInfo.PostProcessBlendWeight = PostProcessBlendWeight;
	InOutInfo.PostProcessSettings = PostProcessSettings;
}

void FNxBindToViewTarget::ApplyToViewInfo(FMinimalViewInfo& InOutInfo) const
{
	if (!bUseCustomPOV)
	{
		if (ViewTarget->IsA(ACameraActor::StaticClass()))
		{
			ACameraActor* cameraActor = dynamic_cast<ACameraActor*>(ViewTarget);
			if (cameraActor != nullptr)
			{
				UCameraComponent* cameraComponent = cameraActor->GetCameraComponent();

				InOutInfo.Location = cameraComponent->GetComponentLocation();
				InOutInfo.Rotation = cameraComponent->GetComponentRotation();
				InOutInfo.FOV = cameraComponent->FieldOfView;
				InOutInfo.OrthoWidth = cameraComponent->OrthoWidth;
				InOutInfo.OrthoNearClipPlane = cameraComponent->OrthoNearClipPlane;
				InOutInfo.OrthoFarClipPlane = cameraComponent->OrthoFarClipPlane;
				InOutInfo.AspectRatio = cameraComponent->AspectRatio;
				InOutInfo.bConstrainAspectRatio = cameraComponent->bConstrainAspectRatio;
				InOutInfo.bUseFieldOfViewForLOD = cameraComponent->bUseFieldOfViewForLOD;
				InOutInfo.ProjectionMode = cameraComponent->ProjectionMode;
				InOutInfo.PostProcessBlendWeight = cameraComponent->PostProcessBlendWeight;
				InOutInfo.PostProcessSettings = cameraComponent->PostProcessSettings;
				return;
			}
		}
	}

	CustomPOV.CopyToViewInfo(InOutInfo);

	if (ViewTarget != nullptr)
	{
		InOutInfo.Location = ViewTarget->GetActorLocation();
		InOutInfo.Rotation = ViewTarget->GetActorRotation();
	}
}