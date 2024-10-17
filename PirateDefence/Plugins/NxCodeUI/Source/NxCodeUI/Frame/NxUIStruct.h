// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "NxUIEnum.h"
#include "NxUIStruct.generated.h"

/**
 *
 */
USTRUCT()
struct NXCODEUI_API FNxFrameUnit
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = CodeUI)
	uint32 MoniterIndex = 0;
	UPROPERTY(EditAnywhere, Category = CodeUI)
	int32 WindowPosX = 0;
	UPROPERTY(EditAnywhere, Category = CodeUI)
	int32 WindowPosY = 0;
	UPROPERTY(EditAnywhere, Category = CodeUI)
	int32 WindowSizeX = 1280;
	UPROPERTY(EditAnywhere, Category = CodeUI)
	int32 WindowSizeY = 720;
	UPROPERTY(EditAnywhere, Category = CodeUI, Meta = (ToolTip = "Fullscreen(0), WindowedFullscreen(1), Windowed(2), NumWindowModes(3)"))
	int32 WindowsMode = 2;
	UPROPERTY(EditAnywhere, Category = CodeUI)
	TSoftClassPtr<class UNxFrameWidget> SubclassFrame;
};

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class NXCODEUI_API UNxFrameArray : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = CodeUI)
	TArray<FNxFrameUnit> FrameInfos;
};


/**
 *
 */
USTRUCT()
struct NXCODEUI_API FNxButtonAniInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	E_ButtonEvent ButtonEvent = E_ButtonEvent::Clicked;

	UPROPERTY(EditAnywhere)
	FString	Name;

	UPROPERTY(EditAnywhere)
	float StartAtTime = 0.f;

	//UPROPERTY(EditAnywhere)
	//float EndAtTime;

	UPROPERTY(EditAnywhere)
	int32 NumberOfLoops = 1;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EUMGSequencePlayMode::Type> PlayMode = EUMGSequencePlayMode::Type::Forward;

	UPROPERTY(EditAnywhere)
	float PlaybackSpeed = 1.0;
};