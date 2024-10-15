
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxLoadingSplashScreen;
struct FNxCenterLayoutSettings;

/**
 * FVxCenterLayoutSettings
 */
class SCenterLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SCenterLayout) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FNxLoadingSplashScreen& Settings, const FNxCenterLayoutSettings& LayoutSettings);
};
