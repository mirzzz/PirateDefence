
#pragma once

#include "SLoadingScreenLayout.h"

struct FNxLoadingSplashScreen;
struct FNxLetterboxLayoutSettings;

/**
 * Letterbox layout loading screen
 */
class SLetterboxLayout : public SLoadingScreenLayout
{
public:
	SLATE_BEGIN_ARGS(SLetterboxLayout) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FNxLoadingSplashScreen& settings, const FNxLetterboxLayoutSettings& LayoutSettings);
};
