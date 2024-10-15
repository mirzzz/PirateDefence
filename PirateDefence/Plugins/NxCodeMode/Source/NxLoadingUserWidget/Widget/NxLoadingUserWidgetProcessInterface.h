// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NxLoadingUserWidgetProcessInterface.generated.h"

/** 로딩 화면 표시를 위한 인터페이스 */
UINTERFACE(BlueprintType)
class NXLOADINGUSERWIDGET_API UNxLoadingUserWidgetProcessInterface : public UInterface
{
	GENERATED_BODY()
};


class NXLOADINGUSERWIDGET_API INxLoadingUserWidgetProcessInterface
{
	GENERATED_BODY()

public:

	// 인터페이스 구현, 현재 로딩 화면을 표시해야 하는지 여부 확인.
	static bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};
