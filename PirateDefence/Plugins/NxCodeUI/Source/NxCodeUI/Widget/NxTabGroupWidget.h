// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxUserWidget.h"
#include "NxTabGroupWidget.generated.h"

class UNxButtonTab;
class UCommonAnimatedSwitcher;

/**
 * 
 */
UCLASS()
class NXCODEUI_API UNxTabGroupWidget : public UNxUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void OnTableButtonClick(uint8 InTabPos);

	virtual void SetTab(uint8 InTabPos, bool InButtonEvent = false);

	uint8	GetTabPos(UNxButtonTab* ButtonTab);

protected:
	UPROPERTY(BlueprintReadOnly, Category = Widget)
	uint8 TabPos;

	UPROPERTY(BlueprintReadOnly, Category = Widget, meta = (BindWidgetOptional))
	TObjectPtr<UCommonAnimatedSwitcher> Switcher;

	TArray<TObjectPtr<UNxButtonTab>> TabButtons;
};
