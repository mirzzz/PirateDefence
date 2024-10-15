
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxAsyncLoadingScreen;
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
	void Construct(const FArguments& InArgs, const FNxAsyncLoadingScreen& settings, const FNxSidebarLayoutSettings& LayoutSettings);
};
