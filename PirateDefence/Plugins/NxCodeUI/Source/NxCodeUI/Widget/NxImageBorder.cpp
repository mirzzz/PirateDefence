// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxImageBorder.h"
#include "Styling/SlateBrush.h"

void UNxImageBorder::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (nullptr != ResourceObject)
	{
		FSlateBrush newBrush = Background;
		newBrush.SetResourceObject(ResourceObject);
		SetBrush(newBrush);
	}

	//if (nullptr != Texture)
	//	SetBrushFromTexture(Texture);
}