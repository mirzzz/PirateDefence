// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxButtonBase.h"
#include "NxTabListWidgetBase.h"
#include "NxButtonTab.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class NXCODEUI_API UNxButtonTab : public UNxButtonBase
{
	GENERATED_BODY()

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	// End of UCommonButtonBase interface

	void CollectionChild() override;

	void RefreshButtonIcon() override;
	void RefreshButtonText() override;

	void Init(uint8 InTabId, bool InSelecte);
	void Selecte();
	void Unselecte();
	
public:
	uint8 TabId;
	TArray<TObjectPtr<class UCommonLazyImage>> Image_Icons;
	TArray<TObjectPtr<class UCommonTextBlock>> Text_Names;
};
