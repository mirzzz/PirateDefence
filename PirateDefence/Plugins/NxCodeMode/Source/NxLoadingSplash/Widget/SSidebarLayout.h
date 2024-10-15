
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxLoadingSplashScreen;
struct FNxSidebarLayoutSettings;

/**
 * Sidebar layout loading screen
 */
class SSidebarLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SSidebarLayout) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FNxLoadingSplashScreen& settings, const FNxSidebarLayoutSettings& LayoutSettings);
};
