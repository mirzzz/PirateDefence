// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonBorder.h"
#include "NxBrushBorder.generated.h"

/**
 * 
 */
UCLASS(Config = NxCodeUI, DefaultConfig, ClassGroup = UI, meta = (Category = "Nx", DisplayName = "NxBrushBorder"))
class NXCODEUI_API UNxBrushBorder : public UCommonBorder
{
	GENERATED_BODY()
	
public:
	virtual void OnWidgetRebuilt() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Nx|Border")
	FSlateBrush Brush;
};
