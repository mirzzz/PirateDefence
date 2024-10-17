// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

NXCODEMODEEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeModeEditor, Log, All);

// Log On:1/Off:0 적용
#define Show_LogCodeModeEditor 1

class FNxCodeModeEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;	

};


