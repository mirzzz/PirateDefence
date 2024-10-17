// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

NXCODEUIEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeUIEditor, Log, All);

// Log On:1/Off:0 Àû¿ë
#define Show_LogCodeUIEditor 1

class FNxCodeUIEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;	

};


