// Fill out your copyright notice in the Description page of Project Settings.

#include "NxProgressIndicatorDialog.h"

void UNxProgressIndicatorDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxProgressIndicatorDialog::SetContext(const FString& InContext)
{
	if (nullptr != Text_Context)
	{
		Text_Context->SetText(FText::FromString(InContext));
	}
}