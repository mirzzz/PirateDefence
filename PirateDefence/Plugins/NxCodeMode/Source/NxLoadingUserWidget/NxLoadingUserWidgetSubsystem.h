// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "UObject/WeakInterfacePtr.h"
#include "Widget/NxLoadingUserWidget.h"
#include "Widget/NxLoadingUserWidgetProcessInterface.h"
#include "NxLoadingUserWidgetSubsystem.generated.h"

class SWidget;
class IInputProcessor;
class IVxWidgetLoadingProcessInterface;
struct FWorldContext;

/**
 * 로딩 화면 표시/숨기기 처리
 *	- Todo : 레벨 로딩시 ProgressBar 연출을 어떻게 해야 할지는 확인 필요.
 *			 비동기 로드시 진행률을 얻는 함수는 GetAsyncLoadPercentage()가 있는데, 
 *			 OpenLevel() 사용시 진행율 얻을수 없고, Seamless Travel 일때만 사용 가능.
 */
UCLASS()
class NXLOADINGUSERWIDGET_API UNxLoadingUserWidgetSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const
	{
		return DebugReasonForShowingOrHidingLoadingScreen;
	}

	// 로딩 화면이 현재 표시되고 있으면 True 반환
	bool GetLoadingScreenDisplayStatus() const
	{
		return bCurrentlyShowingCustomLoading;
	}

	// 로딩 화면 가시성이 변경될 때 호출됩니다.
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWidgetLoadingVisibilityChangedDelegate, bool);
	FORCEINLINE FOnWidgetLoadingVisibilityChangedDelegate& OnWidgetLoadingVisibilityChangedDelegate() { return OnLoadingVisibilityChanged; }

	void RegisterLoadingProcessor(TScriptInterface<INxLoadingUserWidgetProcessInterface> Interface);
	void UnregisterLoadingProcessor(TScriptInterface<INxLoadingUserWidgetProcessInterface> Interface);
	
	void SetCustomLoadingLayout(struct FNxLoadingWidgetLayoutInfo& info);

private:

	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* World);
	void HandleLevelAddedToWorld(ULevel* Level, UWorld* World);
	void HandleLevelRemovedFromWorld(ULevel* Level, UWorld* World);


	// 로딩 화면을 표시할지 숨길지 결정합니다. 매 프레임마다 호출됩니다.
	void UpdateWidgetLoading();

	// 로딩 화면을 표시해야 하는 경우 true 반환.
	bool CheckForAnyNeedToShowCustomLoading();

	// 로딩 화면을 표시하려면 true 를 반환 (필요하거나 다른 이유로 인위적으로 강제 실행하는 경우).
	bool ShouldShowCustomLoading();

	// 화면을 사용하기 위한 초기 로드 플로우에 있는 경우는 true
	bool IsShowingInitialCustomLoading() const;
		
	// 로딩 화면을 표시합니다. 뷰포트에 로드 화면 위젯 설정
	void ShowWidgetLoading();

	// 로드 화면을 숨깁니다. 로드 화면 위젯이 삭제됩니다.
	void HideWidgetLoading();

	// 뷰포트에서 위젯을 제거합니다.
	void RemoveWidgetFromViewport();

	// 로딩 화면이 보이는 동안 게임 내에서 입력이 사용되는 것을 방지합니다.
	void StartBlockingInput();

	// 차단된 경우 게임 내 입력을 다시 시작합니다.
	void StopBlockingInput();

	void ChangePerformanceSettings(bool bEnabingLoadingScreen);

private:

	FDelegateHandle		HWorld_LevelAddedToWorld;				// World Level Add
	FDelegateHandle		HWorld_LevelRemovedFromWorld;			// World Level Remove	

	// 로딩 화면 가시성이 변경될 때 Delegate
	FOnWidgetLoadingVisibilityChangedDelegate OnLoadingVisibilityChanged;

	// 표시하고 있는 로딩 화면 위젯에 대한 참조(있는 경우)
	TSharedPtr<SWidget> LoadingWidget;

	// 로딩 화면이 표시되는 동안 모든 입력을 처리하는 입력 프로세서
	TSharedPtr<IInputProcessor> InputPreProcessor;

	// 외부 로딩 프로세서, 컴포넌트는 로딩을 지연시키는 액터들 처리.
	TArray<TWeakInterfacePtr<INxLoadingUserWidgetProcessInterface>> ExternalLoadingProcessors;

	// 로딩 화면이 뜨는 이유 (설명)
	FString DebugReasonForShowingOrHidingLoadingScreen;

	// 로딩 화면을 표시하기 시작한 시간
	double TimeCustomLoadingShown = 0.0;

	// 로드 화면을 마지막으로 닫고 싶은 시간
	double TimeCustomLoadingLastDismissed = -1.0;

	// 로드 화면이 여전히 표시되는 이유에 대한 다음 로그까지의 시간
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	// PreLoadMap과 PostLoadMap 사이에 있을 때 True 
	bool bCurrentlyInLoadMap = false;

	// 로딩 화면이 현재 표시되고 있는 경우 True
	bool bCurrentlyShowingCustomLoading = false;

	// 로딩 화면 정보
	FNxLoadingWidgetLayoutInfo WidgetLoadingLayoutInfo;
};
