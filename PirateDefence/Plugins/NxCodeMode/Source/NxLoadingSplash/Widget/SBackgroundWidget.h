
#pragma once

#include "Widgets/SCompoundWidget.h"

struct FNxBackgroundSettings;
class FDeferredCleanupSlateBrush;

/**
 * Background widget
 */
class SBackgroundWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBackgroundWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FNxBackgroundSettings& Settings);

private:
	TSharedPtr<FDeferredCleanupSlateBrush> ImageBrush;
};
