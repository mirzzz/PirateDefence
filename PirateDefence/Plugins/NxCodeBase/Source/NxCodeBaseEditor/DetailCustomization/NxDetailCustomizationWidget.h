// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * ANxDetailCustomizationActor의 CustomProperty 생성
 */
class NXCODEBASEEDITOR_API FNxDetailCustomizationWidget : public IDetailCustomization
{
public:
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

	// Combo 관련 함수.
	TSharedRef<SWidget> OnComboMakeMenu();
	void OnComboChange(FString SelectCombo);

protected:	

	TArray<TWeakObjectPtr<UObject>>	OwnerObjects;

	FString	Comobo_Selected;
};
