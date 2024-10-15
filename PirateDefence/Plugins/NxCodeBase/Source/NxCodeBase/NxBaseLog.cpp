﻿// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxBaseLog.h"

// Plugin
DEFINE_LOG_CATEGORY(LogBase);
DEFINE_LOG_CATEGORY(LogState);
DEFINE_LOG_CATEGORY(LogTable);
DEFINE_LOG_CATEGORY(LogNetwork);
DEFINE_LOG_CATEGORY(LogLoading);
DEFINE_LOG_CATEGORY(LogPatcher);
DEFINE_LOG_CATEGORY(LogMode);
DEFINE_LOG_CATEGORY(LogUI);

// Project
DEFINE_LOG_CATEGORY(LogGame);
DEFINE_LOG_CATEGORY(LogEditor);
DEFINE_LOG_CATEGORY(LogPawn);
DEFINE_LOG_CATEGORY(LogCall);
DEFINE_LOG_CATEGORY(LogAsset);
DEFINE_LOG_CATEGORY(LogLogic);
DEFINE_LOG_CATEGORY(LogSound);
DEFINE_LOG_CATEGORY(LogPhysic);
//DEFINE_LOG_CATEGORY(LogInput);	// UE 정의되어있음
DEFINE_LOG_CATEGORY(LogUser);

// Development - R/D
DEFINE_LOG_CATEGORY(LogCheat);
DEFINE_LOG_CATEGORY(LogDev);


#if PLATFORM_WINDOWS
	DEFINE_LOG_CATEGORY(LogWin);
#endif 
