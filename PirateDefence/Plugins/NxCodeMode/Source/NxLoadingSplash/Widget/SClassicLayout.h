
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxAsyncLoadingScreen;
struct FNxClassicLayoutSettings;

/**
 * FVxClassicLayoutSettings
 */
class SClassicLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SClassicLayout) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FNxAsyncLoadingScreen& settings, const FNxClassicLayoutSettings& LayoutSettings);
};
