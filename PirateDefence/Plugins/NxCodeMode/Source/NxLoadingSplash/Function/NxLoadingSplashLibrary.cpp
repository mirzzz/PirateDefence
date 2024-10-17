
#include "NxLoadingSplashLibrary.h"
#include "MoviePlayer.h"
#include "NxLoadingSplashModule.h"
 
int32 UNxLoadingSplashLibrary::DisplayBackgroundIndex	= -1;
int32 UNxLoadingSplashLibrary::DisplayTipTextIndex	= -1;
int32 UNxLoadingSplashLibrary::DisplayMovieIndex		= -1;
bool  UNxLoadingSplashLibrary::bShowLoadingScreen		= true;

void UNxLoadingSplashLibrary::SetDisplayBackgroundIndex(int32 BackgroundIndex)
{
	UNxLoadingSplashLibrary::DisplayBackgroundIndex = BackgroundIndex;
}

void UNxLoadingSplashLibrary::SetDisplayTipTextIndex(int32 TipTextIndex)
{
	UNxLoadingSplashLibrary::DisplayTipTextIndex = TipTextIndex;
}

void UNxLoadingSplashLibrary::SetDisplayMovieIndex(int32 MovieIndex)
{
	UNxLoadingSplashLibrary::DisplayMovieIndex = MovieIndex;	
}

void UNxLoadingSplashLibrary::SetEnableLoadingScreen(bool bIsEnableLoadingScreen)
{
	bShowLoadingScreen = bIsEnableLoadingScreen;
}

void UNxLoadingSplashLibrary::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void UNxLoadingSplashLibrary::PreloadBackgroundImages()
{
	if (FNxLoadingSplashModule::IsAvailable())
	{
		FNxLoadingSplashModule& LoadingScreenModule = FNxLoadingSplashModule::Get();
		if (LoadingScreenModule.IsPreloadBackgroundImagesEnabled())
		{
			LoadingScreenModule.LoadBackgroundImages();
		}		
	}
}

void UNxLoadingSplashLibrary::RemovePreloadedBackgroundImages()
{
	if (FNxLoadingSplashModule::IsAvailable())
	{
		FNxLoadingSplashModule& LoadingScreenModule = FNxLoadingSplashModule::Get();
		LoadingScreenModule.RemoveAllBackgroundImages();
	}
}

