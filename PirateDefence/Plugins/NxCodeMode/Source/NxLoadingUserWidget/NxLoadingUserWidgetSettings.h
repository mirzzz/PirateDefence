
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MoviePlayer.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Styling/SlateBrush.h"
#include "Framework/Text/TextLayout.h"
#include "NxBaseEnum.h"
#include "NxLoadingUserWidgetSettings.generated.h"


USTRUCT(BlueprintType)
struct NXLOADINGUSERWIDGET_API FNxLoadingUserWidgetScreen
{
	GENERATED_BODY()	

	// 로딩 화면에 로드할 위젯 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingWidget;

	// 뷰포트 스택에 있는 로딩 화면 위젯의 zOrder
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Display)
	int32 LoadingZOrder = 10000;

	// 텍스처 스트리밍이 흐릿함을 피할 기회를 주기 위해 다른 로딩이 끝난 후 로딩 화면을 유지하는 데 걸리는 시간(초)
	// 참고: 이는 일반적으로 반복 시간 동안 에디터에 적용되지 않지만 HoldCustomLoadingAdditionalSecsEvenInEditor를 통해 활성화할 수 있습니다.
 	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Configuration, meta=(ForceUnits=s, ConsoleVariable="Nx.LoadingUserWidget.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingAdditionalSecs = 2.0f;

	// 로딩 화면이 영구적으로 중단된 것으로 간주되는 초 단위의 간격입니다(0이 아닌 경우).
 	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LoadingHeartbeatHangDuration = 0.0f;

	// 로딩 화면을 유지하는 각 로그 사이의 초 단위 간격(0이 아닌 경우).
 	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LogLoadingHeartbeatInterval = 5.0f;

	// true인 경우 로딩 화면이 표시되거나 숨겨지는 이유가 매 프레임마다 로그에 인쇄됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="Nx.LoadingUserWidget.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingReasonEveryFrame = 0;

	// 로딩 화면 강제 표시 (디버깅에 유용)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="Nx.LoadingUserWidget.AlwaysShow"))
	bool ForceLoadingVisible = false;

	// 에디터에서도 추가 HoldCustomLoadingAdditionalSecs 지연을 적용해야 할지 여부(로딩 화면에서 반복할 때 유용함)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Debugging)
	bool HoldLoadingAdditionalSecsEvenInEditor = false;

	// 에디터에서도 추가 HoldCustomLoadingAdditionalSecs 지연을 적용해야 할지 여부(로딩 화면에서 반복할 때 유용함)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Configuration)
	bool ForceTickCustomLoadingEvenInEditor = true;
};


/**
 * LoadingUserWidget Settings 
 */
UCLASS(Config = Plugin, DefaultConfig, meta = (DisplayName = "LoadingSettings"))
class NXLOADINGUSERWIDGET_API UNxLoadingUserWidgetSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UNxLoadingUserWidgetSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// 프로젝트 Category 설정
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { /*return FApp::GetProjectName();*/ return FName("Nx"); }
	virtual FName GetSectionName() const override { return FName("NxLoadingUserWidget"); }

#if WITH_EDITOR
	// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:

	// 레벨 로딩시 LoadingScreen 노출 여부 (AsyncLoading.bUseAsyncLoading이 True 일경우, WidgetLoading.bUseWidgetLoading을 우선한다)
	UPROPERTY(Config, EditAnywhere, Category = "Widget")
	bool bUseWidgetLoading = true;


	// 새로운 레벨을 열 때마다 나타나는 기본 로딩 화면-Custom 버전
	UPROPERTY(Config, EditAnywhere, Category = "Widget", meta = (EditCondition = "bUseWidgetLoading==True"))
	FNxLoadingUserWidgetScreen WidgetLoadingSettings;
};
