
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// 참고 URL:
// - https://gitlab.com/UEMarketplaceSubmission_Public/multiwindows4ue4_public - Plugin Demo Project
// - https://github.com/Batname/SpectatorWindow/tree/master - Spectator Window

/*
1. 지원 Class 매핑 적용

	[/Script/Engine.Engine]
	GameEngine=/Script/NxCodeWindows.NxCodeWindowsGameEngine	
	GameViewportClientClassName=/Script/NxCodeWindows.NxCodeWindowsGameViewportClient
	LocalPlayerClassName=/Script/NxCodeWindows.NxCodeWindowsLocalPlayer
	UnrealEdEngine=/Script/NxCodeWindowsEditor.NxCodeWindowsUnrealEdEngine
*/

NXCODEWINDOWS_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeWindows, Log, All);

// Log On:1/Off:0 적용
#define Show_LogCodeWindows 1

/*
	엔진 및 Viewport, LocalPlayer Wrap
*/
class NXCODEWINDOWS_API FNxCodeWindowsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:

	void GetMultiWindowsManager(class UNxCodeWindowsManager*& MultiWindowsManager);

private:

	int32 MultiWindowsManagerObjectIndex = 0;
};
