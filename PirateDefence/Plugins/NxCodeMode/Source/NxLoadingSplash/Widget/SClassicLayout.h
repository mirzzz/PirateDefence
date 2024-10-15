
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxLoadingSplashScreen;
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
	void Construct(const FArguments& InArgs, const FNxLoadingSplashScreen& settings, const FNxClassicLayoutSettings& LayoutSettings);
};
