// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/EnumRange.h"
#include "NxUIEnum.generated.h"


UENUM(BlueprintType)
enum class E_CommonInputMode : uint8
{
	GameAndMenu,
	Game,
	Menu,
	None,
};

/**
 * UI 레이어 분류
 */
UENUM(BlueprintType)
enum class E_StackLayer : uint8
{
	Game,
	GameMenu,
	Menu,
	Dialog,
	Notices,
	Indicator,
	Dev,
	Max
};

/**
 * UI 에 대한 비동기 로드 작업의 상태입니다.
 */
enum class E_AsyncWidgetLayerState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};

UENUM()
enum class E_ScreenOrientation : uint8
{
	Portrait,
	Landscape
};


// 버튼 이벤트
UENUM()
enum class E_ButtonEvent : uint8
{
	Clicked,
	DoubleClicked,
	Pressed,
	Released,
	Hovered,
	Unhovered,
	FocusReceived,
	FocusLost,
};

// Dialog 반환 결과
UENUM(BlueprintType)
enum class E_DialogResult : uint8
{
	// "yes" 버튼 클릭
	Confirmed,
	// "no" 버튼 클릭
	Declined,
	// "ignore/cancel" 버튼 클릭
	Cancelled,
	// 대화창 명시적 종료 (사용자 입력 없음).
	Killed,

	Unknown UMETA(Hidden)
};

// 위젯 아이디
UENUM()
enum class E_WidgetID : uint32
{
	None = 0,	
	// RowName으로 사용할 경우
	UseRawName,

	// 사용자 정의 UI
	
};

// 위젯을 어느곳에 부착할지 여부
UENUM(BlueprintType)
enum class E_WidgetAttachMonitor : uint8
{
	First,		
	Second,		
	Third,		
};

namespace Nx_StringTAG
{
	static const FName AniFadeIn	= TEXT("OnFadeIn");
	static const FName AniFadeOut	= TEXT("OnFadeOut");
	static const FName AniMoveLeft	= TEXT("OnMoveLeft");
	static const FName AniMoveRight = TEXT("OnMoveRight");
}