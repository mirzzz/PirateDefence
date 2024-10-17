
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraTypes.h"
#include "NxCodeView.generated.h"

/**
 * 특정 표시 플래그 설정을 얻기 위해 보기 모드를 정의합니다
	(일부는 켜짐, 일부 꺼짐 및 일부는 변경되지 않음).
 * 순서를 변경하지 마십시오. ID는 편집기로 직렬화됩니다.
 */
UENUM(BlueprintType)
enum class E_ViewModeType : uint8
{
	/** Wireframe w/ brushes. */
	VMI_BrushWireframe = 0,
	/** Wireframe w/ BSP. */
	VMI_Wireframe = 1,
	/** Unlit. */
	VMI_Unlit = 2,
	/** Lit. */
	VMI_Lit = 3,
	VMI_Lit_DetailLighting = 4,
	/** Lit wo/ materials. */
	VMI_LightingOnly = 5,
	/** Colored according to light count. */
	VMI_LightComplexity = 6,
	/** Colored according to shader complexity. */
	VMI_ShaderComplexity = 8,
	/** Colored according to world-space LightMap texture density. */
	VMI_LightmapDensity = 9,
	/** Colored according to light count - showing lightmap texel density on texture mapped objects. */
	VMI_LitLightmapDensity = 10,
	VMI_ReflectionOverride = 11,
	VMI_VisualizeBuffer = 12,
	//	VMI_VoxelLighting = 13,

	/** Colored according to stationary light overlap. */
	VMI_StationaryLightOverlap = 14,

	VMI_CollisionPawn = 15,
	VMI_CollisionVisibility = 16,
	//VMI_UNUSED = 17,
	/** Colored according to the current LOD index. */
	VMI_LODColoration = 18,
	/** Colored according to the quad coverage. */
	VMI_QuadOverdraw = 19,
	/** Visualize the accuracy of the primitive distance computed for texture streaming. */
	VMI_PrimitiveDistanceAccuracy = 20,
	/** Visualize the accuracy of the mesh UV densities computed for texture streaming. */
	VMI_MeshUVDensityAccuracy = 21,
	/** Colored according to shader complexity, including quad overdraw. */
	VMI_ShaderComplexityWithQuadOverdraw = 22,
	/** Colored according to the current HLOD index. */
	VMI_HLODColoration = 23,
	/** Group item for LOD and HLOD coloration*/
	VMI_GroupLODColoration = 24,
	/** Visualize the accuracy of the material texture scales used for texture streaming. */
	VMI_MaterialTextureScaleAccuracy = 25,
	/** Compare the required texture resolution to the actual resolution. */
	VMI_RequiredTextureResolution = 26,

	VMI_Max UMETA(Hidden),

	VMI_Unknown = 255 UMETA(Hidden),
};

/**
 *
 */
UENUM(BlueprintType)
enum class E_ViewPointType : uint8
{
	CustomViewPoint,
	BindToPlayerController,
	BindToViewTarget
};
 
USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct FNxViewInfo
{
public:
	GENERATED_USTRUCT_BODY()

	FNxViewInfo()
		: Location(ForceInit)
		, Rotation(ForceInit)
		, FOV(90.0f)
		, OrthoWidth(512.0f)
		, OrthoNearClipPlane(0.0f)
		, OrthoFarClipPlane(WORLD_MAX)
		, AspectRatio(1.33333333f)
		, bConstrainAspectRatio(false)
		, bUseFieldOfViewForLOD(true)
		, ProjectionMode(ECameraProjectionMode::Perspective)
		, PostProcessBlendWeight(0.0f)
	{
	}

public:

	// 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector Location;

	// (x:롤 y; 피치; z: 요). 단위: 도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FRotator Rotation;

	// 원근 모드의 시야각(각도) - 직교 모드에서는 무시됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
	float FOV;

	// 직교 뷰의 원하는 너비(World 단위) - 원근 모드에서는 무시됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float OrthoWidth;

	// 직교 뷰의 먼 평면 거리(World 단위)
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = Camera)
	float OrthoNearClipPlane;

	// 
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = Camera)
	float OrthoFarClipPlane;

	// 종횡비(너비/높이); bConstrainAspectRatio가 참이 아니면 무시됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (ClampMin = "0.1", ClampMax = "100.0", EditCondition = "bConstrainAspectRatio"))
	float AspectRatio;

	// bConstrainAspectRatio가 true인 경우 대상 보기의 종횡비가 이 카메라가 요청한 것과 다른 경우 검은색 막대가 추가됨.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	uint32 bConstrainAspectRatio : 1;

	// 참이면 메시에 사용할 세부 수준을 계산할 때 시야각을 고려하십시오.
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = CameraSettings)
	uint32 bUseFieldOfViewForLOD : 1;

	// 카메라의 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode;

	// PostProcessSettings를 적용해야 하는지 여부?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (UIMin = "0.0", UIMax = "1.0"))
	float PostProcessBlendWeight;

	// PostProcessBlendWeight가 0이 아닌 경우 사용할 후처리 설정입니다.
	UPROPERTY(BlueprintReadWrite, Category = Camera)
	struct FPostProcessSettings PostProcessSettings;

	void CopyToViewInfo(FMinimalViewInfo& InOutInfo) const;
};

USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct NXCODEWINDOWS_API FNxViewPoint
{
public:
	GENERATED_BODY()

	//  Camera POV. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	FNxViewInfo CustomPOV;
};


USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct NXCODEWINDOWS_API FNxBindToPlayerController
{
public:
	GENERATED_USTRUCT_BODY()
};



USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct NXCODEWINDOWS_API FNxBindToViewTarget
{
public:
	GENERATED_USTRUCT_BODY()

	FNxBindToViewTarget() :ViewTarget(nullptr), bUseCustomPOV(true)
	{

	}

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	AActor* ViewTarget;

	// bUseCustomPOV가 false이고 ViewTarget이 카메라 액터인 경우 카메라 액터의 POV를 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	uint32 bUseCustomPOV : 1;

	// bUseCustomPOV가 참이면 이 POV를 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews", meta = (EditCondition = "bUseCustomPOV"))
	FNxViewInfo CustomPOV;

	void ApplyToViewInfo(FMinimalViewInfo& InOutInfo) const;
};

/**
 * 
 */
USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct NXCODEWINDOWS_API FNxCodeView
{	
public:
	GENERATED_USTRUCT_BODY()

	FNxCodeView() :
		LocationAndSizeOnScreen(0.0f, 0.0f, 0.5f, 0.5f), LocationOffsetOfViewpoint(ForceInit),
		RotationOffsetOfViewpoint(ForceInit), ViewpointType(E_ViewPointType::BindToPlayerController)
	{
	}

	~FNxCodeView()
	{
	}

public:
	// 뷰 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	FName Name;

	// LocationX, LocationY, Width, Height
	//	- LocationX: [0.0~1.0]; LocationY: [0.0~1.0]; Width: [0.0~1.0]; Height: [0.0~1.0].
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	FVector4 LocationAndSizeOnScreen;

	// 위치 오프셋 추가.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	FVector LocationOffsetOfViewpoint;

	// 회전 오프셋 추가.
	//	- (x:Roll y; Pitch; z: Yaw). Units: deg. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	FRotator RotationOffsetOfViewpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews")
	E_ViewPointType ViewpointType;

	/** Used when ViewpointType == E_ViewPointType::CustomViewPoint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews", meta = (EditCondition = "ViewpointType==E_ViewPointType::CustomViewPoint"))
	FNxViewPoint CustomViewPoint;

	/** Used when ViewpointType == E_ViewPointType::BindToPlayerController. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews", meta = (EditCondition = "ViewpointType==E_ViewPointType::BindToPlayerController"))
	FNxBindToPlayerController BindToPlayerController;

	/** Used when ViewpointType == E_ViewPointType::BindToViewTarget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiViews", meta = (EditCondition = "ViewpointType==E_ViewPointType::BindToViewTarget"))
	FNxBindToViewTarget BindToViewTarget;
};
