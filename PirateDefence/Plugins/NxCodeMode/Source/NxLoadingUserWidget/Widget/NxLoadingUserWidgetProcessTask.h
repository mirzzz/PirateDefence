// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/WeakInterfacePtr.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "NxLoadingUserWidgetProcessInterface.h"
#include "NxLoadingUserWidgetProcessTask.generated.h"

UCLASS(BlueprintType)
class NXLOADINGUSERWIDGET_API UNxLoadingUserWidgetProcessTask : public UObject, public INxLoadingUserWidgetProcessInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"))
	static UNxLoadingUserWidgetProcessTask* CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

public:
	UNxLoadingUserWidgetProcessTask() { }

	UFUNCTION(BlueprintCallable)
	void Unregister();

	UFUNCTION(BlueprintCallable)
	void SetShowLoadingScreenReason(const FString& InReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	
	FString Reason;
};