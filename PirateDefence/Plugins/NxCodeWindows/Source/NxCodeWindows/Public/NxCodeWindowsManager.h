
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxCodeWindow.h"
#include "NxCodeWindowsManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubWindowsArrayChanged, const UNxCodeWindowsManager*, MultiWindowsManager);

/**
 * 멀티 윈도우 관리자.
 */
UCLASS(BlueprintType)
class NXCODEWINDOWS_API UNxCodeWindowsManager : public UObject
{
	GENERATED_BODY()
	
public:

	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "CreateWindow", Keywords = "Create Window"), Category = "VxPlugIn|MultiWindows|Window")
	//void CreateWindow(FText WindowTitle, FVector2D WindowPosition = FVector2D(500.0f, 500.0f), FVector2D WindowSize = FVector2D(1280.0f, 720.0f));
	 
	// 새창 생성
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CreateGameViewportClientWindow", Keywords = "Create GameViewportClient Window"), Category = "VxPlugIn|MultiWindows|Window")
	UNxCodeWindow* CreateGameViewportClientWindow();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "GetMainWindow", Keywords = "Get Main Window"), Category = "VxPlugIn|MultiWindows|Window")
	void GetMainWindow(UNxCodeWindow*& OutMainWindow);

	void UpdateWorldContentBeforeTick(TIndirectArray<FWorldContext>& WorldList);
	void UpdateWorldContentAfterTick(TIndirectArray<FWorldContext>& WorldList);

private:

	TSharedPtr<SWindow> CreateWindow_Internal(FText WindowTitle, FVector2D WindowPosition = FVector2D(500.0f, 500.0f), FVector2D WindowSize = FVector2D(1280.0f, 720.0f));

	// 게임 뷰포트 창 생성.
	void CreateSceneViewport(UNxCodeWindow* Window);

	// 슬레이트 UI가 추가된 뷰포트 위젯 생성.
	void CreateViewportWidget(UNxCodeWindow* Window);

	FSceneViewport* GetSceneViewport(UGameViewportClient* ViewportClient) const;

public:
	UPROPERTY()
	bool bLoad = false;

	UPROPERTY()
	int32 NumOfNewViewportWindow = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NxCodeWindows")
	TArray<UNxCodeWindow*> SubWindows;

	UPROPERTY()
	bool bAddedNewWindow = false;

	UPROPERTY(BlueprintAssignable, Category = "NxCodeWindows|Delegate")
	FOnSubWindowsArrayChanged OnSubWindowsArrayChanged;

private:

	UPROPERTY()
	UNxCodeWindow* MainWindow;

	UPROPERTY()
	TArray<UGameViewportClient*> TempGameViewportClients;

	TIndirectArray<FWorldContext> TempWorldList;
};
