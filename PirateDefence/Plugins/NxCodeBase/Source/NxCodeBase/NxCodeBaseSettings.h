// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NxCodeBaseSettings.generated.h"

/**
 * Common Plugin Setting
 */
UCLASS(config = Plugin, DefaultConfig, meta = (DisplayName = "CodeBaseSettings"))
class NXCODEBASE_API UNxCodeBaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ~UDeveloperSettings
	UNxCodeBaseSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetContainerName() const override { return FName("Project"); }	// 프로젝트 설정
	virtual FName GetCategoryName() const override { /*return FApp::GetProjectName();*/ return FName("Nx"); }
	virtual FName GetSectionName() const override { return FName("NxCodeBase"); }

#if WITH_EDITOR
	// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

public:

};
