// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * ANxDetailCustomizationActor�� CustomProperty ����
 */
class NXCODEBASEEDITOR_API FNxDetailCustomizationWidget : public IDetailCustomization
{
public:
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

	// Combo ���� �Լ�.
	TSharedRef<SWidget> OnComboMakeMenu();
	void OnComboChange(FString SelectCombo);

protected:	

	TArray<TWeakObjectPtr<UObject>>	OwnerObjects;

	FString	Comobo_Selected;
};
