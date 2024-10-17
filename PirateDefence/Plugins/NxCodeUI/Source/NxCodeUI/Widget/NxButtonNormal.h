// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxButtonBase.h"
#include "NxButtonNormal.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class NXCODEUI_API UNxButtonNormal : public UNxButtonBase
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
	
public:
	//UPROPERTY(BlueprintReadOnly, Category = "UI.ButtonNormal", meta = (BindWidget))
	//TObjectPtr<class UCommonLazyImage> Image_Icon;

	//UPROPERTY(BlueprintReadOnly, Category = "UI.ButtonNormal", meta = (BindWidget))
	//TObjectPtr<class UCommonTextBlock> Text_Name;

	TArray<TObjectPtr<class UCommonLazyImage>> Image_Icons;
	TArray<TObjectPtr<class UCommonTextBlock>> Text_Names;
};
