
#include "STipWidget.h"
#include "NxLoadingSplashSettings.h"
#include "Widgets/Text/STextBlock.h"
#include "NxFunc_LoadingSplash.h"

void STipWidget::Construct(const FArguments& InArgs, const FNxTipSettings& Settings)
{
	if (Settings.TipText.Num() > 0)
	{
		int32 TipIndex = FMath::RandRange(0, Settings.TipText.Num() - 1);
		
		if (Settings.bSetDisplayTipTextManually == true)
		{			
			if (Settings.TipText.IsValidIndex(UNxFunc_LoadingSplash::GetDisplayTipTextIndex()))
			{
				TipIndex = UNxFunc_LoadingSplash::GetDisplayTipTextIndex();
			}
		}

		ChildSlot
		[
			SNew(STextBlock)		
			.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
			.Font(Settings.Appearance.Font)
			.ShadowOffset(Settings.Appearance.ShadowOffset)
			.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
			.Justification(Settings.Appearance.Justification)
			.WrapTextAt(Settings.TipWrapAt)
			.Text(Settings.TipText[TipIndex])			
		];
		
	}
}
