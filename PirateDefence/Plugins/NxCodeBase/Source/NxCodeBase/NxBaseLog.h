// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

// 대상 헤더가 Windows.h 또는 WinSDK 헤더에 종속된 경우 이 헤더를 먼저 포함합니다.
//#include "Windows/MinWindows.h"

// 타사 헤더를 포함할 때 충돌 기호와 관련된 컴파일 오류가 있는 경우 다음 헤더 및 매크로를 사용하십시오
//#include "Windows/AllowWindowsPlatformTypes.h"
//#include "Windows/HideWindowsPlatformTypes.h"

// Plugin
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogBase,		Log, All);			
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogState,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogTable,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogNetwork,	Log, All);		// UE 정의되어있음		
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogLoading,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogPatcher,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogUI,		Log, All);

// Project
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogGame,		Log, All);		
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogEditor,	Log, All);		
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogPawn,		Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogCall,		Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogAsset,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogLogic,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogSound,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogPhysic,	Log, All);
//NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogInput,	Log, All);		// UE 정의되어있음
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogUser,		Log, All);

// Developer User Define
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogCheat,	Log, All);
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogDev,		Log, All);		// 개발

// Platform
#if PLATFORM_WINDOWS
NXCODEBASE_API DECLARE_LOG_CATEGORY_EXTERN(LogWin,		Log, All);
#endif 

/*
ELogVerbosity
	Fatal			콘솔 (및 로그 파일)에 치명적인 오류 및 충돌을 항상 인쇄 (로깅이 비활성화 된 경우에도)
	Error			콘솔 (및 로그 파일)에 오류를 인쇄합니다.
	Warning			콘솔 (및 로그 파일)에 경고를 인쇄합니다.
	Display			콘솔 (및 로그 파일)에 메시지를 인쇄합니다.
	Log				로그 파일에 메시지를 인쇄합니다 (콘솔에 인쇄하지 않음).
	Verbose			자세한 메시지를 로그 파일에 인쇄합니다 (특정 카테고리에 대해 자세한 로깅을 사용하는 경우, 일반적으로 자세한 로깅에 사용됨).
	VeryVerbose		자세한 메시지를 로그 파일로 인쇄합니다 (VeryVerbose 로깅을 사용하는 경우 일반적으로 출력을 스팸하는 자세한 로깅에 사용됩니다.)
*/

// Log On:1/Off:0 적용

// Plugin
#define Show_LogBase		1		// Common, 기반
#define Show_LogUI			1		// UE 정의	
#define Show_LogNet			1	
#define Show_LogGolf		1	 
#define Show_LogPatch		1
#define Show_LogLoading		1
#define Show_LogMultiWin	1

// Project
#define Show_LogGame		1	
#define Show_LogEditor		1	
#define Show_LogPawn		1	
#define Show_LogCall		1	
#define Show_LogAsset		1	
#define Show_LogLogic		1	
#define Show_LogSound		1	
#define Show_LogPhysic		1	
#define Show_LogInput		1		// UE 정의
#define Show_LogUser		1	


// Development - R/D
#define Show_LogCheat		1
#define Show_LogDev			1	


#if PLATFORM_WINDOWS
	#define Show_LogWin		1	
#endif 

// function(line.no)
#define NX_LINE									(FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__)+ TEXT(")"))

// (if) 함수 
#define NxLog(Category, Verb, Format, ...)		if (Show##_##Category) UE_LOG(Category, Verb, TEXT("%s .....%s"), *FString::Printf(Format, ##__VA_ARGS__ ),  *NX_LINE )
#define NxLogFunc(Category, Verb)				if (Show##_##Category) UE_LOG(Category, Verb, TEXT(".....%s"), *NX_LINE)
#define NxLogText(Category, Verb, Text)			if (Show##_##Category) UE_LOG(Category, Verb, TEXT("%s .....%s"), Text, *NX_LINE )


#define NxPrintFunc(Category)					NxLogFunc(Category,		Log)
#define NxPrintText(Category, Text)				NxLogText(Category,		Log,		Text)

#define NxPrintLog(Category, Format, ...)		NxLog(Category,			Log,		TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintDisplay(Category, Format, ...)	NxLog(Category,			Display,	TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintVerbose(Category, Format, ...)	NxLog(Category,			Verbose,	TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintTodo(Category, Format, ...)		NxLog(Category,			Warning,	TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintWarning(Category, Format, ...)	NxLog(Category,			Warning,	TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintError(Category, Format, ...)		NxLog(Category,			Error,		TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))
#define NxPrintFatal(Category, Format, ...)		NxLog(Category,			Fatal,		TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__ ))

#define NxMsg(Color, Time, Text)				GEngine->AddOnScreenDebugMessage(-1, Time, Color, Text)
#define NxMsgArgs(Color, Time, Format, ...)		GEngine->AddOnScreenDebugMessage(-1, Time, Color, *FString::Printf(Format, ##__VA_ARGS__ ))

// ex > NxCheckReturn( obj != nullptr );
// ex > NxCheckReturn( obj != nullptr, false );
#define NxCheckReturn(Condition, ...) \
{ \
	if(!(Condition)) \
	{ \
		VxPrintError(LogBase, TEXT("[CheckReturn Failed] : %s"), TEXT(""#Condition"")); \
		return __VA_ARGS__; \
	} \
}