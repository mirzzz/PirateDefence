// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonActivatableWidget.h"
#include "NxCodeUILibrary.generated.h"
 
 // UI 지원 함수.
UCLASS() 
class NXCODEUI_API UNxCodeUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UNxCodeUILibrary() { }
	 
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = CodeUI, meta = (WorldContext = "WidgetContextObject"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);
	
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = CodeUI, meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = CodeUI, meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	//UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	//static UCommonActivatableWidget* PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	//UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	//static void PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	//UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	//static void PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	static void OpenAsync(TSubclassOf<UCommonActivatableWidget> ActivatableWidget, uint8 FramePos, uint8 eLayer);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	static ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	// 입력신호 정지 시킴.
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	static FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	// 입력신호 복귀 시킴.
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	static void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);




	//------------------------------------------------------------------- 미니맵 관련 코드

	// 라인 교차 판정
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = CodeUI)
	static bool LineIntersection2D(UPARAM(ref) FVector2D& A, UPARAM(ref) FVector2D& A2, UPARAM(ref) FVector2D& B, UPARAM(ref) FVector2D& B2, UPARAM(ref) FVector2D& Out);
	

	// 입력된 라인 점정을 윈도우 사이즈로 클립한후, 한쌍씩 라인을 추가해준다. (최대 라인의 두배 크기로 Out 잔환)
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = CodeUI)
	static void BoundWindowClipLine(UPARAM(ref) TArray<FVector2D>& Lines, UPARAM(ref) FVector2D& WindowExtend, UPARAM(ref) TArray<FVector2D>& OutLine );


private:
	static int32 InputSuspensions;
};