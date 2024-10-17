// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

NXCODEBASEPREV_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeBasePrev, Log, All);

// Log On:1/Off:0 적용
#define Show_LogCodeBasePrev 1

class FNxCodeBasePrevModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

