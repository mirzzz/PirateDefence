
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxCodeViewManager.h"
#include "Blueprint/UserWidget.h"
#include "NxCodeWindow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCloseWindow, const UNxCodeWindow*, NxCodeWindow);

/**
 * 윈도우 창 지원
 */
UCLASS(BlueprintType)
class NXCODEWINDOWS_API UNxCodeWindow : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClearViews", Keywords = "ClearViews"), Category = "NxCodeWindows")
	void ClearViews();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetAsCustomViewMode", Keywords = "SetAsCustomViewMode"), Category = "NxCodeWindows")
	void SetAsCustomViewMode(const FNxCodeViewManager& InViewManager);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetAsNormalViewMode", Keywords = "SetAsNormalViewMode"), Category = "NxCodeWindows")
	void SetAsNormalViewMode();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle(NormalViewMode|MultiViewsMode)", Keywords = "ToggleNormalViewModeAndMultiViewsMode"), Category = "NxCodeWindows")
	void ToggleNormalViewModeAndMultiViewsMode(const bool IsNormalViewMode);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IsNormalViewModeOrMultiViewsMode", Keywords = "IsNormalViewModeOrMultiViewsMode"), Category = "NxCodeWindows")
	void IsNormalViewModeOrMultiViewsMode(bool& IsNormalViewMode);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetWindowIndex", Keywords = "GetWindowIndex"), Category = "NxCodeWindows")
	void GetWindowIndex(int32& WindowIndex, bool& IsValid);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ResizeWindow", Keywords = "Resize Window"), Category = "NxCodeWindows")
	void ResizeWindow(int32 ResX = 1280, int32 ResY = 720, EWindowMode::Type WindowMode = EWindowMode::Type::Windowed);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CloseWindow", Keywords = "Close Window"), Category = "NxCodeWindows")
	void CloseWindow();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetWindowTitle", Keywords = "Set Window Title"), Category = "NxCodeWindows")
	void SetWindowTitle(FText WindowTitle = FText());
		
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetWindowPosition", Keywords = "Set Window Position"), Category = "NxCodeWindows")
	void SetWindowPosition(FVector2D NewPosition = FVector2D(100.0f, 100.0f));
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetWindowPosition", Keywords = "Get Window Position"), Category = "NxCodeWindows")
	void GetWindowPosition(FVector2D& WindowPosition);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetWindowSize", Keywords = "Get Window Size"), Category = "NxCodeWindows")
	void GetWindowSize(FVector2D& WindowSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetWindowTitle", Keywords = "Set Window Title"), Category = "NxCodeWindows")
	FText GetWindowTitle();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetRectWindow", Keywords = "SetRectWindow"), Category = "NxCodeWindows")
	void SetRectWindow(FVector2D WindowPosition, int32 ResX = 1280, int32 ResY = 720, EWindowMode::Type WindowMode = EWindowMode::Type::Windowed);

	// 창 닫힐때 호출됨.
	void OnGameWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed);

	// 창 이동시에 호출됨.
	void OnGameWindowMoved(const TSharedRef<SWindow>& WindowBeingMoved);

	void OnViewportResized(FViewport* Viewport, uint32 Unused);
	
	void OnLevelRemovedFromWorldAndRemoveWidgetsInViewport(ULevel* InLevel, UWorld* InWorld);

public:

	// 현재 게임 인스턴스를 나타내는 뷰 포트입니다. 
	UPROPERTY()
	UGameViewportClient* GameViewportClient;

public:

	// 게임 뷰포트 창
	TWeakPtr<class SWindow> GameViewportClientWindow;
	 
	// 기본 Scene 뷰포트
	TSharedPtr<class FSceneViewport> SceneViewport;


	// 게임 뷰포트 위젯
	TSharedPtr<class SViewport> ViewportWidget;

	UPROPERTY()
	TArray<UUserWidget*> UserWidgetsInViewport;

	UPROPERTY(BlueprintAssignable, Category = "NxCodeWindow|Delegate")
	FOnCloseWindow OnCloseWindow;
	 
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows")
	FNxCodeViewManager ViewManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows|ViewManager")
	bool DoNotShowAnyView = false;
};
