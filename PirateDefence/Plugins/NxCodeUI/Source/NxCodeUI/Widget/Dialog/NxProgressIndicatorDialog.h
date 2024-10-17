// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxPopupWindow.h"
#include "Components/CircularThrobber.h"
#include "CommonTextBlock.h"
#include "NxProgressIndicatorDialog.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class NXCODEUI_API UNxProgressIndicatorDialog : public UNxPopupWindow
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:
	void SetContext(const FString& InContext);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Vx.ProgressIndicator", meta = (BindWidget))
	TObjectPtr<UCircularThrobber> CircularThrobber;

	UPROPERTY(BlueprintReadOnly, Category = "Vx.ProgressIndicator", meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Context;
};
