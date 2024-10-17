
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NxCodeWindowsManager.h"
#include "GenericPlatform/GenericWindow.h"
#include "Blueprint/UserWidget.h"
#include "NxCodeWindowsLibrary.generated.h"

/* 
*	
*/
UCLASS()
class NXCODEWINDOWS_API UNxCodeWindowsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
 
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMultiWindowsManager", Keywords = "GetMultiWindowsManager"), Category = "VxPlugIn|MultiWindows|Manager")
	static void GetMultiWindowsManager(UNxCodeWindowsManager*& MultiWindowsManager);

	/**
	 * 명시적으로 크기를 설정하기 위해 SetDesiredSizeInViewport가 호출되지 않는 한 UserWidget을 하나의 창에 추가하고 전체 화면을 채웁니다.
	 *	Note: Window 에서 UserWidget을 제거하려면 "RemoveWidgetFromWindow()"를 사용하십시오.
	 *
	 * @param ZOrder	: 숫자가 높을수록 위젯이 위에 그려진다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (DisplayName = "AddWidgetToWindow", Keywords = "Add Widget To Window" ,AdvancedDisplay = "ZOrder"), Category = "VxPlugIn|MultiWindows|Widget")
	static void AddWidgetToWindow(UUserWidget* UserWidget, UNxCodeWindow* Window, int32 ZOrder);
	
	// AddWidgetToWindow() 로 추가된 Widget 창을 제거.
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "MultiWindows|Widget")
	static void RemoveWidgetFromWindow(UUserWidget* UserWidget);
};
