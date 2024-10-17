
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "SceneView.h"
#include "NxCodeViewManager.h"
#include "NxCodeWindowsLocalPlayer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCodeWindowsLocalPlayer, Log, All);
#define Show_LogCodeWindowsLocalPlayer 1

/**
* 멀티 뷰 생성 지원
*/
UCLASS(ClassGroup = "NxCodeWindows", BlueprintType, Blueprintable, meta = (ShortTooltip = "NxCodeWindowsLocalPlayer is use to show multi views."))
class NXCODEWINDOWS_API UNxCodeWindowsLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()
public:
	

public:
	virtual void PostInitProperties() override;
	
	// 객체를 파괴하기 전에 호출됩니다. 객체를 파괴하기로 결정하는 즉시 호출되어 객체가 시작할 수 있도록 합니다. 비동기식 프로세스 정리.
	virtual void BeginDestroy() override;
	virtual void FinishDestroy() override;

	// 플레이어의 시점 검색
	//	- @param OutViewInfo - 반환 시 플레이어에 대한 보기 정보가 포함됩니다.
	virtual void GetViewPoint(FMinimalViewInfo& OutViewInfo) const override;

	/**
	 * 뷰 액터에서 그리기 위한 뷰 설정 계산
	 *
	 * @param	View - output view struct
	 * @param	OutViewLocation - output actor location
	 * @param	OutViewRotation - output actor rotation
	 * @param	Viewport - current client viewport
	 * @param	ViewDrawer - optional drawing in the view
	 * @param	StereoViewIndex - index of the view when using stereoscopy
	 */
	virtual FSceneView* CalcSceneView(class FSceneViewFamily* ViewFamily,
		FVector& OutViewLocation,
		FRotator& OutViewRotation,
		FViewport* Viewport,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE) override;

	/**
	 * 프로젝션에 필요한 다양한 데이터 비트 도출을 위한 도우미 함수
	 *
	 * @param	Viewport				The ViewClient's viewport
	 * @param	ProjectionData			The structure to be filled with projection data
	 * @param	StereoViewIndex		    The index of the view when using stereoscopy
	 * @return  False if there is no viewport, or if the Actor is null
	 */
	virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex = INDEX_NONE) const override;
	
	virtual void OffsetViewLocationAndRotation(FMinimalViewInfo& InOutViewInfo) const;
	
	/** FExec interface
	*/
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	/**
	* Exec command handlers
	*/

	/**
	* 뷰 액터에서 그리기 위한 뷰 초기화 설정을 계산합니다.
	*
	* @param	OutInitOptions - output view struct. Not every field is initialized, some of them are only filled in by CalcSceneView
	* @param	Viewport - current client viewport
	* @param	ViewDrawer - optional drawing in the view
	* @param	StereoPass - whether we are drawing the full viewport, or a stereo left / right pass
	* @return	true if the view options were filled in. false in various fail conditions.
	*/
	bool CalcSceneViewInitOptions_Custom(
		struct FSceneViewInitOptions& OutInitOptions,
		FViewport* Viewport,
		const int32 IndexOfView,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE);

	virtual FSceneView* CalcMultiViews(class FSceneViewFamily* ViewFamily,
		FVector& OutViewLocation,
		FRotator& OutViewRotation,
		FViewport* Viewport,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE);

	virtual FSceneView* CalcView_Custom(class FSceneViewFamily* ViewFamily,
		FVector& OutViewLocation,
		FRotator& OutViewRotation,
		FViewport* Viewport,
		const FNxCodeView ViewSetting,
		const int32 IndexOfView,
		class FViewElementDrawer* ViewDrawer = NULL,
		int32 StereoViewIndex = INDEX_NONE);

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMaxNumOfViews"), Category = "NxCodeWindows")
	int32 GetMaxNumOfViews()
	{
		return MaxNumOfViews;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows")
	bool EnableMultiViews = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows")
	FNxCodeViewManager ViewManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows")
	E_ViewModeType ViewMode;

	static const int32 MaxNumOfViews;

	TArray<FSceneViewStateReference> ViewStates;
	 
private:
	
	int32 CurrentViewIndex = 0;
};
