
#pragma once

#include "SLoadingWidget.h"

struct FNxLoadingWidgetSettings;

/**
 * 
 */
class SHorizontalLoadingWidget : public SLoadingWidget
{
public:
	SLATE_BEGIN_ARGS(SHorizontalLoadingWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FNxLoadingWidgetSettings& settings);
};
