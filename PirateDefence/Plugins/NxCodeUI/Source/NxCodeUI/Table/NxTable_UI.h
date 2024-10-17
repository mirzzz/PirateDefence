// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NxUIEnum.h"
#include "NxFrameWidget.h"
#include "NxTable_UI.generated.h"

/*
 * UI 정보
 */
USTRUCT(BlueprintType)
struct FNxTable_UI : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	// 위젯 아이디
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	E_WidgetID WidgetID = E_WidgetID::None;

	// 위젯 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	FString WidgetDesc;

	// 위젯 Class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	TSoftClassPtr<class UNxActivatableWidgetWindow>	WidgetClass;
	
	// 부착될 Monitor 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	E_WidgetAttachMonitor	AttachFrame = E_WidgetAttachMonitor::First;
	
	// 부착될 Layer 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	E_StackLayer AttachLayer = E_StackLayer::Game;

	// 재조정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	bool OverridePosition = false;

	// 위치 재조정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, Meta = (EditCondition = "OverridePosition", EditConditionHides))
	FIntPoint OverridePos  = FIntPoint::ZeroValue;

	// 크기 재조정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, Meta = (EditCondition = "OverridePosition", EditConditionHides))
	FIntPoint OverrideSize = FIntPoint::ZeroValue;


	// 생성자
	FNxTable_UI() {}

	// 복사 생성자
	FNxTable_UI(FNxTable_UI const& Other)
	{
		*this = Other;
	}
};


