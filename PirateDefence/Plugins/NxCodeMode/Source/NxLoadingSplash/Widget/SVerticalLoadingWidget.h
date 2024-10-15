
#pragma once

#include "SLoadingWidget.h"

/**
 * 
 */
class SVerticalLoadingWidget : public SLoadingWidget
{
public:
	SLATE_BEGIN_ARGS(SVerticalLoadingWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FNxLoadingWidgetSettings& settings);
};
