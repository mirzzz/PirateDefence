// Fill out your copyright notice in the Description page of Project Settings.


#include "NxGameInstanceBase.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "GameFramework/GameUserSettings.h"
#include "NxWorldSubsystemBase.h"
#include "NxWorldPartitionSubsystemBase.h"
#include "Table/NxTableSubsystem.h"
#include "NxBaseLog.h"


UNxGameInstanceBase* UNxGameInstanceBase::BaseInstance = nullptr;


UNxGameInstanceBase::UNxGameInstanceBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{

}

#if WITH_EDITOR
FGameInstancePIEResult UNxGameInstanceBase::InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params)
{
	FGameInstancePIEResult result = Super::InitializeForPlayInEditor(PIEInstanceIndex, Params);

	return result;
}

FGameInstancePIEResult UNxGameInstanceBase::StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params)
{
	FGameInstancePIEResult result = Super::StartPlayInEditorGameInstance(LocalPlayer, Params);

	return result;
}
#endif 

void UNxGameInstanceBase::Init()
{
	NxPrintTxt(LogBase, TEXT("[----------------------------------------------------------------------------------]"));
	NxPrintTxt(LogBase, TEXT("[ NxGameInstanceBase Init                                                          ]"));
	NxPrintTxt(LogBase, TEXT("[----------------------------------------------------------------------------------]"));

	Super::Init();

	// 언리얼 Define 정의 확인용 Log.
	InitPrintDefineLog();

	BaseInstance = this;
		
	RegistEventdetection();
}

void UNxGameInstanceBase::Shutdown()
{
	UnregistEventdetection();

	BaseInstance = nullptr;

	Super::Shutdown();

	NxPrintTxt(LogBase, TEXT("[----------------------------------------------------------------------------------]"));
	NxPrintTxt(LogBase, TEXT("[ NxGameInstanceBase Shutdown                                                      ]"));
	NxPrintTxt(LogBase, TEXT("[----------------------------------------------------------------------------------]"));
}

void UNxGameInstanceBase::InitPrintDefineLog()
{
	FString Summary = "DefineBuild Summary : ";
	NxPrintTxt(LogBase, TEXT("[ DefineBuild -------------------------------------------------------------------- ]"));

#if PLATFORM_WINDOWS	
	NxPrintTxt(LogBase, TEXT("[ DefineBuild --------------------------------------------------- PLATFORM_WINDOWS ]"));
	Summary += "Windows = ";
#elif PLATFORM_ANDROID
	NxPrintTxt(LogBase, TEXT("[ DefineBuild --------------------------------------------------- PLATFORM_ANDROID ]"));
	Summary += "Android = ";
#elif PLATFORM_IOS
	NxPrintTxt(LogBase, TEXT("[ DefineBuild ------------------------------------------------------- PLATFORM_IOS ]"));
	Summary += "IOS = ";
#endif

	/*
	Debug		: 디버그 엔진 모듈 + 디버그 게임 모듈
	DebugGame	: 릴리즈 엔진 모듈 + 디버그 게임 모듈
	Development	: 릴리즈 엔진 모듈 + 릴리즈 게임 모듈

	DegugGameEditor,   DebugGame	: 게임 코드에 대해 활성화된 디버그 기호 활성 (릴리즈 엔진 모듈 과 디버그 게임 모듈로 컴파일 함)
	DevelopmentEditor, Development	: 게임 코드에 대해 활성화된 디버그 기호 비활성
	*/
	EBuildConfiguration buildConfig = FApp::GetBuildConfiguration();

	switch (buildConfig)
	{
		case EBuildConfiguration::Debug:
			NxPrintTxt(LogBase, TEXT("[ DefineBuild ----------------------------------------------------- UE_BUILD_DEBUG ]"));
			Summary += "Debug, ";
			break;
		case EBuildConfiguration::DebugGame:			
			NxPrintTxt(LogBase, TEXT("[ DefineBuild ----------------------------------------------- UE_BUILD_DEVELOPMENT ]"));
			Summary += "DebugGame, ";
			break;
		case EBuildConfiguration::Development:
			NxPrintTxt(LogBase, TEXT("[ DefineBuild ----------------------------------------------- UE_BUILD_DEVELOPMENT ]"));
			Summary += "Development, ";
			break;
		case EBuildConfiguration::Shipping:
			NxPrintTxt(LogBase, TEXT("[ DefineBuild -------------------------------------------------- UE_BUILD_SHIPPING ]"));
			Summary += "Shipping, ";
			break;
		case EBuildConfiguration::Test:
			NxPrintTxt(LogBase, TEXT("[ DefineBuild ------------------------------------------------------ UE_BUILD_TEST ]"));
			Summary += "Test, ";
			break;
	}

#if UE_GAME
	// DebugGame, Development	
	NxPrintTxt(LogBase, TEXT("[ DefineBuild ------------------------------------------------------------ UE_GAME ]"));
	Summary += "Game, ";
#endif 

#if UE_EDITOR
	// DebugGameEditor, DevelopmentEditor	
	NxPrintTxt(LogBase, TEXT("[ DefineBuild ---------------------------------------------------------- UE_EDITOR ]"));
	Summary += "Editor, ";
#endif 

	EBuildTargetType buildTarget = FApp::GetBuildTargetType();
	switch (buildTarget)
	{
	case EBuildTargetType::Game:
		Summary += "BuildTarget:Game";
		break;
	case EBuildTargetType::Server:
		Summary += "BuildTarget:Server";
		break;
	case EBuildTargetType::Client:
		Summary += "BuildTarget:Client";
		break;
	case EBuildTargetType::Editor:
		Summary += "BuildTarget:Editor";
		break;
	case EBuildTargetType::Program:
		Summary += "BuildTarget:Program";
		break;
	}
	NxPrintLog(LogBase, TEXT("[ %s "), *Summary);
	
	NxPrintTxt(LogBase, TEXT("[ DefinePaths -------------------------------------------------------------------- ]"));
	// LaunchDir의 절대경로를 기준으로 나머지는 모두 상대경로로 구성되어 있다.
	NxPrintLog(LogBase, TEXT("[ Path::LaunchDir--------------------Absolute > %s"), *FPaths::LaunchDir());
	NxPrintLog(LogBase, TEXT("[ Path::RootDir----------------------Absolute > %s"), *FPaths::RootDir());
	
	NxPrintLog(LogBase, TEXT("[ Path::EngineDir---------------------------- > %s "), *FPaths::EngineDir());
	NxPrintLog(LogBase, TEXT("[ Path::EngineConfigDir---------------------- > %s "), *FPaths::EngineConfigDir());
	NxPrintLog(LogBase, TEXT("[ Path::EnginePluginsDir--------------------- > %s "), *FPaths::EnginePluginsDir());
	NxPrintLog(LogBase, TEXT("[ Path::EngineSavedDir---------------Absolute > %s "), *FPaths::EngineSavedDir());
																		    
	NxPrintLog(LogBase, TEXT("[ Path::ProjectDir--------------------------- > %s "), *FPaths::ProjectDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectUserDir----------------------- > %s "), *FPaths::ProjectUserDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectConfigDir--------------------- > %s "), *FPaths::ProjectConfigDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectContentDir-------------------- > %s "), *FPaths::ProjectContentDir());
	NxPrintLog(LogBase, TEXT("[ Path::GameDevelopersDir-------------------- > %s "), *FPaths::GameDevelopersDir());
	NxPrintLog(LogBase, TEXT("[ Path::GameUserDeveloperDir----------------- > %s "), *FPaths::GameUserDeveloperDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectSavedDir---------------------- > %s "), *FPaths::ProjectSavedDir());
	NxPrintLog(LogBase, TEXT("[ Path::BugItDir----------------------------- > %s "), *FPaths::BugItDir());
	NxPrintLog(LogBase, TEXT("[ Path::ScreenShotDir------------------------ > %s "), *FPaths::ScreenShotDir());
	NxPrintLog(LogBase, TEXT("[ Path::VideoCaptureDir---------------------- > %s "), *FPaths::VideoCaptureDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectPersistentDownloadDir--------- > %s "), *FPaths::ProjectPersistentDownloadDir());
	NxPrintLog(LogBase, TEXT("[ Path::ProjectPluginsDir-------------------- > %s "), *FPaths::ProjectPluginsDir());
}

void UNxGameInstanceBase::ReturnToMainMenu()
{
	// Online 세션 종료, 추후 모든것을 재 설정해야 한다.

	Super::ReturnToMainMenu();
}


// Convenience Static Function		
//------------------------------------------------------------------
// 각 Subsystem에 static Get() 함수를 통해 접근 지원.
UNxTableSubsystem*			UNxGameInstanceBase::TableSubsystem()			{ return GetGameInstanceSubsystem<UNxTableSubsystem>(); }
UNxWorldSubsystemBase*			UNxGameInstanceBase::WorldSubsystem()			{ return GetWorldSubSystem<UNxWorldSubsystemBase>(); }
UNxWorldPartitionSubsystemBase*	UNxGameInstanceBase::WorldPartitionSubsystem()	{ return GetWorldSubSystem<UNxWorldPartitionSubsystemBase>(); }


// Event Function		
//------------------------------------------------------------------
void UNxGameInstanceBase::RegistEventdetection()
{
	if(!bRegistEvent)
		return;

	// 틱 이벤트
	H_Ticker					= FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::OnEvent_Ticker), 0.0f);

	// 레벨 이벤트
	H_PreLoadMapWithContext		= FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::OnEvent_PreLoadMap);
	H_PostLoadMapWithWorld		= FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnEvent_PostLoadMap);

	// 앱 이벤트
	H_AppHasReactivated			= FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(this, &ThisClass::OnEvent_AppHasReactivated);
	H_AppWillDeactivate			= FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &ThisClass::OnEvent_AppWillDeactivate);
	H_AppHasEnteredForeground	= FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &ThisClass::OnEvent_AppHasEnteredForeground);
	H_AppWillEnterBackground	= FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &ThisClass::OnEvent_AppWillEnterBackground);
	H_AppLicenseChange			= FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &ThisClass::OnEvent_AppLicenseChange);

	// 해상도/컨트롤러
	H_SafeFrameChangedEvent		= FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &ThisClass::OnEvent_SafeFrameChangedEvent);
	
	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	H_ControllerConnectionChange = DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::OnEvent_ControllerConnectionChange);


#if WITH_EDITOR
	// 엑터 변경 감지 (Editor-only)
	H_ActorSpawned = GEngine->OnLevelActorAdded().AddUObject(this, &UNxGameInstanceBase::OnEvent_ActorSpawned);
	H_ActorDeleted = GEngine->OnLevelActorDeleted().AddUObject(this, &UNxGameInstanceBase::OnEvent_ActorDeleted);
	H_LevelActorListChanged = GEngine->OnLevelActorListChanged().AddUObject(this, &UNxGameInstanceBase::OnEvent_LevelActorListChanged);
#endif 
}

void UNxGameInstanceBase::UnregistEventdetection()
{
	if (!bRegistEvent)
		return;

	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

#if WITH_EDITOR
	// 엑터 변경 감지 (Editor Only)
	GEngine->OnLevelActorAdded().Remove(H_ActorSpawned);
	GEngine->OnLevelActorDeleted().Remove(H_ActorDeleted);
	GEngine->OnLevelActorListChanged().Remove(H_LevelActorListChanged);
#endif 
}


bool UNxGameInstanceBase::OnEvent_Ticker(float deltaTime)
{
	// false 반환할 경우, tick 종료
	return true;
}

void UNxGameInstanceBase::OnEvent_PreLoadMap(const FWorldContext& InWorldContext, const FString& InMapName)
{
	NxPrintLog(LogBase, TEXT("OnEvent_PostLoadMap() [%s]"), *InMapName);
}

void UNxGameInstanceBase::OnEvent_PostLoadMap(UWorld* InLoadedWorld)
{
	if (InLoadedWorld)
	{
		NxPrintLog(LogBase, TEXT("OnEvent_PostLoadMap() [%s]"), *InLoadedWorld->GetName());
	}
}


// 플랫폼에서 Application 활성화 됨. 
//	- 콘솔, 모바일, Window 등에서 뒤로 숨어 있다가, 메인 화면으로 나올때 발생하는 이벤트
void UNxGameInstanceBase::OnEvent_AppHasReactivated()
{
	NxPrintTxt(LogBase, TEXT("API 활성화"));
}

// 플랫폼에서 Application 비활성화 됨. 
//	- 콘솔, 모바일, Window 등에서 메인 화면에 출력 중이었다가, 뒤로 숨을 경우 발생 하는 이벤트
void UNxGameInstanceBase::OnEvent_AppWillDeactivate()
{
	NxPrintTxt(LogBase, TEXT("API 비활성화!"));
}

// 앞단(Front)으로 돌아옴.
void UNxGameInstanceBase::OnEvent_AppHasEnteredForeground()
{
	NxPrintTxt(LogBase, TEXT("API Foreground 진입!"));
}

// 뒷단(Back)로 돌어감.
void UNxGameInstanceBase::OnEvent_AppWillEnterBackground()
{
	NxPrintTxt(LogBase, TEXT("API Background 진입!"));
}

// 인증 변경
void UNxGameInstanceBase::OnEvent_AppLicenseChange()
{
	NxPrintTxt(LogBase, TEXT("인증 변경"));
}

// 해상도 변경
void UNxGameInstanceBase::OnEvent_SafeFrameChangedEvent()
{
	NxPrintTxt(LogBase, TEXT("해상도 변경"));

	if (OnSafeFrameChangedEventDelegate.IsBound())
	{
		if (GEngine && GEngine->GameViewport)
		{
			TWeakPtr<SWindow>		window	 = GEngine->GameViewport->GetWindow();
			UGameViewportClient*	viewport = GEngine->GameViewport;
			
			if (viewport)
			{
				FIntPoint		 viewSize = viewport->Viewport->GetSizeXY();
				const FVector2D  viewportCenter = FVector2D(viewSize.X/2, viewSize.Y/2);

				OnSafeFrameChangedEventDelegate.Broadcast(viewSize.X, viewSize.Y);
			}
		}

		// GameUserSetting 값으로 해상도 변경.
		//UGameUserSettings* userSettings = GEngine->GetGameUserSettings();
		//if (ensure(userSettings))
		//{
		//	const FIntPoint CurrentRes = userSettings->GetScreenResolution();			
		//	FSystemResolution::RequestResolutionChange(Width, Height, EWindowMode::Windowed);
		//}
	}
}

// 컨트롤러 변경
void UNxGameInstanceBase::OnEvent_ControllerConnectionChange(EInputDeviceConnectionState InConnectionState, FPlatformUserId InPlatformUserId, FInputDeviceId InInputDeviceId)
{
	NxPrintLog(LogBase, TEXT("게임패드 신호 : state:%d, userId:%d, deviceId:%d"),
		*NxEnum_GetString(EInputDeviceConnectionState, InConnectionState), InPlatformUserId.GetInternalId(), InInputDeviceId.GetId() );

	switch(InConnectionState)
	{
	case EInputDeviceConnectionState::Connected:		// 패드 연결됨
		break;

	case EInputDeviceConnectionState::Disconnected:		// 패드 끊어짐
		break;
	}
}

#if WITH_EDITOR

void UNxGameInstanceBase::OnEvent_ActorSpawned(AActor* Actor)
{
	if(Actor)
	{
		//FString msg = FString::Printf(TEXT("NxGameInstanceBase::OnEvent_ActorSpawned() ActorName:%s"), *Actor->GetName());		
		//VxMsg(FColor::White, 5.0f, msg);
		//NxPrintLog(LogBase, TEXT("ActorSpawned() Name:%s"), *Actor->GetName());
	}
}

void UNxGameInstanceBase::OnEvent_ActorDeleted(AActor* Actor)
{
	if (Actor) 
	{
		//FString msg = FString::Printf(TEXT("NxGameInstanceBase::OnEvent_ActorDeleted() ActorName:%s"), *Actor->GetName());
		//VxMsg(FColor::White, 5.0f, msg);
		//NxPrintLog(LogBase, TEXT("ActorDeleted() Name:%s"), *Actor->GetName());
	}
}

void UNxGameInstanceBase::OnEvent_LevelActorListChanged()
{
	//NxPrintFunc(LogBase);
	//VxMsg(FColor::White, 5.0f, TEXT("NxGameInstanceBase::OnEvent_LevelActorListChanged()"));
}
#endif 