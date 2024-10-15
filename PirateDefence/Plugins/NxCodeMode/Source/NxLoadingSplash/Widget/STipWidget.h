
#pragma once

#include "Widgets/SCompoundWidget.h"

struct FNxTipSettings;

/**
 * Tip widget
 */
class STipWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STipWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FNxTipSettings& settings);
};
