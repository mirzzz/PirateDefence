// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

NXCODEBASEEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeBaseEditor, Log, All);

// Log On:1/Off:0 적용
#define Show_LogCodeBaseEditor 1

class FNxCodeBaseEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;	

};


