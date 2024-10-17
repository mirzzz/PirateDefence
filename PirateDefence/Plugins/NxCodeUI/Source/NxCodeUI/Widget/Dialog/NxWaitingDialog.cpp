// Fill out your copyright notice in the Description page of Project Settings.

#include "NxWaitingDialog.h"

void UNxWaitingDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNxWaitingDialog::SetContext(const FString& InContext)
{
	if (nullptr != Text_Context)
	{
		Text_Context->SetText(FText::FromString(InContext));
	}
}