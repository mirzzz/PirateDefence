// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NxCodePatcherLibrary.generated.h"


// PatchSettgings 관련 코드
UCLASS()
class NXCODEPATCHER_API UNxCodePatcherLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:	

	// PatchSettings의 패치 활성화 여부 반환
	UFUNCTION(BlueprintPure)
	static bool GetPatchEnable();
	
	UFUNCTION(BlueprintPure)
	static E_DevelopmentType			GetPatchDevelopType();

	UFUNCTION(BlueprintPure)
	static TSoftClassPtr<UUserWidget>	GetPatchWidgetClass();

	UFUNCTION(BlueprintPure)
	static TSoftObjectPtr<UWorld>		GetNextMapAfterPatching();
};
	