
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxAsyncLoadingScreen;
struct FNxDualSidebarLayoutSettings;

/**
 * Dual Sidebar Layout
 */
class SDualSidebarLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SDualSidebarLayout) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FNxAsyncLoadingScreen& Settings, const FNxDualSidebarLayoutSettings& LayoutSettings);
};
