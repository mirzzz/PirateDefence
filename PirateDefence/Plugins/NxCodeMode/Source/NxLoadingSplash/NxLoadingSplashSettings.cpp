
#include "NxLoadingSplashSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"

#define LOCTEXT_NAMESPACE "NxLoadingSplash"

FNxLoadingWidgetSettings::FNxLoadingWidgetSettings() : LoadingText(LOCTEXT("Loading", "LOADING")) {}
// FVxLoadingCompleteTextSettings::FVxLoadingCompleteTextSettings() : LoadingCompleteText(LOCTEXT("Loading Complete", "Loading is complete! Press any key to continue...")) {}

UNxLoadingSplashSettings::UNxLoadingSplashSettings(const FObjectInitializer& Initializer) : Super(Initializer)
{
	StartupLoadingSettings.TipWidget.TipWrapAt = 1000.0f;
	StartupLoadingSettings.bShowWidgetOverlay  = false;
	DefaultLoadingSettings.TipWidget.TipWrapAt = 1000.0f;

	// 기본 폰트 설정.
	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(TEXT("/Engine/EngineFonts/Roboto"));
		StartupLoadingSettings.TipWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Normal"));
		DefaultLoadingSettings.TipWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 20, FName("Normal"));
		StartupLoadingSettings.LoadingWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 32, FName("Bold"));
		DefaultLoadingSettings.LoadingWidget.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 32, FName("Bold"));
		StartupLoadingSettings.LoadingCompleteTextSettings.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Normal"));
		DefaultLoadingSettings.LoadingCompleteTextSettings.Appearance.Font = FSlateFontInfo(RobotoFontObj.Object, 24, FName("Normal"));
	}
}

#if WITH_EDITOR

FText UNxLoadingSplashSettings::GetSectionText() const
{
	return LOCTEXT("NxLoadingSplashSettingsName", "Plugin NxLoadingSplash");
}

FText UNxLoadingSplashSettings::GetSectionDescription() const
{
	return LOCTEXT("NxLoadingSplashSettingsDescription", "Loading Splash or Loading Level Layout 설정");
}

void UNxLoadingSplashSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNxLoadingSplashSettings, bUseAsyncLoading))
	{
		// SystemType 변경
		if (bUseAsyncLoading == true)
		{
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE


