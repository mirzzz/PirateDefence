
#include "SBackgroundWidget.h"
#include "NxLoadingSplashSettings.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Engine/Texture2D.h"
#include "NxLoadingSplashLibrary.h"
#include "NxLoadingSplashModule.h"

void SBackgroundWidget::Construct(const FArguments& InArgs, const FNxBackgroundSettings& Settings)
{
	// If there's an image defined
	if (Settings.Images.Num() > 0)
	{
		int32 ImageIndex = FMath::RandRange(0, Settings.Images.Num() - 1);

		if (Settings.bSetDisplayBackgroundManually == true)
		{
			if (Settings.Images.IsValidIndex(UNxLoadingSplashLibrary::GetDisplayBackgroundIndex()))
			{
				ImageIndex = UNxLoadingSplashLibrary::GetDisplayBackgroundIndex();
			}
		}		
		
		// Load background from settings
		UTexture2D* LoadingImage = nullptr;
		const FSoftObjectPath& ImageAsset = Settings.Images[ImageIndex];
		UObject* ImageObject = ImageAsset.TryLoad();
		LoadingImage = Cast<UTexture2D>(ImageObject);

		// If IsPreloadBackgroundImagesEnabled is enabled, load from images array
		FNxLoadingSplashModule& LoadingScreenModule = FNxLoadingSplashModule::Get();
		if (LoadingScreenModule.IsPreloadBackgroundImagesEnabled())
		{
			TArray<UTexture2D*> BackgroundImages = LoadingScreenModule.GetBackgroundImages();
			if (!BackgroundImages.IsEmpty() && BackgroundImages.IsValidIndex(ImageIndex))
			{
				LoadingImage = BackgroundImages[ImageIndex];
			}
		}		
		
		if (LoadingImage)
		{
			ImageBrush = FDeferredCleanupSlateBrush::CreateBrush(LoadingImage);
			ChildSlot
			[
				SNew(SBorder)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(Settings.Padding)
				.BorderBackgroundColor(Settings.BackgroundColor)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				[
					SNew(SScaleBox)
					.Stretch(Settings.ImageStretch)
					[
						SNew(SImage)						
						.Image(ImageBrush.IsValid() ? ImageBrush->GetSlateBrush() : nullptr)						
					]
				]
			];			
		}
	}
}