﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxPopupWindow.h"
#include "Components/CircularThrobber.h"
#include "CommonTextBlock.h"
#include "NxWaitingDialog.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class NXCODEUI_API UNxWaitingDialog : public UNxPopupWindow
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

public:
	void SetContext(const FString& InContext);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Nx.Waiting", meta = (BindWidget))
	TObjectPtr<UCircularThrobber> CircularThrobber;

	UPROPERTY(BlueprintReadOnly, Category = "Nx.Waiting", meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Context;
};
