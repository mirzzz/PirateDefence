// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxBrushBorder.h"

#include "Styling/SlateBrush.h"

void UNxBrushBorder::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	SetBrush(Brush);
}