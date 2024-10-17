
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/GameViewportClient.h"
#include "NxCodeWindow.h"
#include "SceneTypes.h"
#include "StereoRendering.h"
#include "CommonGameViewportClient.h"

#include "NxCodeWindowsGameViewportClient.generated.h"

UCLASS()
class NXCODEWINDOWS_API UNxCodeWindowsGameViewportClient : public UCommonGameViewportClient // UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:
	/** DO NOT USE. 이 생성자는 핫 리로드 목적으로만 내부용으로 사용됩니다. */
	UNxCodeWindowsGameViewportClient(FVTableHelper& Helper);

	virtual ~UNxCodeWindowsGameViewportClient();

public:

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	/**
	 * 개체 파괴를 완료하기 위해 호출됩니다. UObject::FinishDestroy 가 호출된 후에는 오브젝트의 메모리에 더 이상 액세스할 수 없습니다.
	 *
	 * @warning 여기에서 속성이 소멸되기 때문에 Super::FinishDestroy()는 자식 클래스의 FinishDestroy() 메서드 시작 부분이 아니라 항상 끝 부분에서 호출되어야 합니다.
	 */
	virtual void FinishDestroy() override;
	//~ End UObject Interface

public:

	//~ Begin FViewportClient Interface.
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;

	//~ End FViewportClient Interface.

public:
	/** CSVProfiler 카메라 통계 업데이트 */
	virtual void UpdateCsvCameraStats(const FSceneView* View);

	/**
	 *  해당 플레이어의 뷰포트 검색
	 * @param OutViewInfo - 반환 시 플레이어에 대한 보기 정보가 포함됩니다.
	 * @param StereoPass  - 뷰포트를 가져올 입체 패스(있는 경우). 여기에는 눈 오프셋이 포함됩니다.
	 */
	virtual void GetViewPoint(class ULocalPlayer* LocalPlayer, const int32 IndexOfView, FMinimalViewInfo& OutViewInfo) const;

	virtual void OffsetViewLocationAndRotation(FMinimalViewInfo& InOutViewInfo, const int32 IndexOfView) const;

	/** 
	 * 이 뷰 액터에서 그리기 위한 뷰 초기화 설정을 계산합니다.
	 *
	 * @param	OutInitOptions - 출력 뷰 구조체. 모든 필드가 초기화되는 것은 아니며 일부 필드는 CalcSceneView에 의해서만 채워집니다.
	 * @param	Viewport - 현재 클라이언트 뷰포트
	 * @param	ViewDrawer - 뷰의 선택적 도면
	 * @param	StereoViewIndex - 입체영상 사용 시 뷰 인덱스
	 * @return	보기 옵션이 채워진 경우 true. 다양한 실패 조건에서 false.
	 */
	virtual bool CalcSceneViewInitOptions(class ULocalPlayer* LocalPlayer,
		struct FSceneViewInitOptions& OutInitOptions,
		FViewport* InViewport,
		const int32 IndexOfView,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE);

	/**
	 * 이 뷰 액터에서 그리기 위한 뷰 설정을 계산합니다.
	 *
	 * @param	View - 출력 뷰 구조체
	 * @param	OutViewLocation - 액터 위치 출력
	 * @param	OutViewRotation - 액터 회전 출력
	 * @param	Viewport - 현재 클라이언트 뷰포트
	 * @param	ViewDrawer - 뷰에서 선택적 드로잉
	 * @param	StereoViewIndex - 입체영상 사용시 뷰인덱스값
	 */
	virtual FSceneView* CalcSceneView(class ULocalPlayer* LocalPlayer,
		class FSceneViewFamily* ViewFamily,
		FVector& OutViewLocation,
		FRotator& OutViewRotation,
		FViewport* InViewport,
		int32 IndexOfView,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE);

public:

	UPROPERTY()
	UNxCodeWindow* Window;

public:
	static const int32 MaxNumOfViews;

private:
	TArray<FSceneViewStateReference> ViewStates;
};
