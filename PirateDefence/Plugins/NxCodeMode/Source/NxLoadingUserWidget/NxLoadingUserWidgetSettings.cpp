
#include "NxLoadingUserWidgetSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"
#include "NxLoadingSplashSettings.h"

#define LOCTEXT_NAMESPACE "NxLoadingUserWidget"


UNxLoadingUserWidgetSettings::UNxLoadingUserWidgetSettings(const FObjectInitializer& Initializer) : Super(Initializer)
{
}

#if WITH_EDITOR

FText UNxLoadingUserWidgetSettings::GetSectionText() const
{
	return LOCTEXT("VNxLoadingUserWidgetSettingsName", "Plugin NxLoadingUserWidget");
}

FText UNxLoadingUserWidgetSettings::GetSectionDescription() const
{
	return LOCTEXT("NxLoadingUserWidgetSettingsDescription", "LoadingUserWidget User Layeout 설정");
}

void UNxLoadingUserWidgetSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UNxLoadingUserWidgetSettings, bUseWidgetLoading))
	{
		UNxLoadingSplashSettings* asyncLoading = GetMutableDefault<UNxLoadingSplashSettings>();
		
		// SystemType 변경
		if (bUseWidgetLoading == true)
		{
			if (asyncLoading->bUseAsyncLoading)
			{
				asyncLoading->bUseAsyncLoading = false;
			}
		}
		else
		{
			asyncLoading->bUseAsyncLoading = true;
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE


