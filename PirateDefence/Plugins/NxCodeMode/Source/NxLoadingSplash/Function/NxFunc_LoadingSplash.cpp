
#include "NxFunc_LoadingSplash.h"
#include "MoviePlayer.h"
#include "NxLoadingSplashModule.h"
 
int32 UNxFunc_LoadingSplash::DisplayBackgroundIndex	= -1;
int32 UNxFunc_LoadingSplash::DisplayTipTextIndex	= -1;
int32 UNxFunc_LoadingSplash::DisplayMovieIndex		= -1;
bool  UNxFunc_LoadingSplash::bShowLoadingScreen		= true;

void UNxFunc_LoadingSplash::SetDisplayBackgroundIndex(int32 BackgroundIndex)
{
	UNxFunc_LoadingSplash::DisplayBackgroundIndex = BackgroundIndex;
}

void UNxFunc_LoadingSplash::SetDisplayTipTextIndex(int32 TipTextIndex)
{
	UNxFunc_LoadingSplash::DisplayTipTextIndex = TipTextIndex;
}

void UNxFunc_LoadingSplash::SetDisplayMovieIndex(int32 MovieIndex)
{
	UNxFunc_LoadingSplash::DisplayMovieIndex = MovieIndex;	
}

void UNxFunc_LoadingSplash::SetEnableLoadingScreen(bool bIsEnableLoadingScreen)
{
	bShowLoadingScreen = bIsEnableLoadingScreen;
}

void UNxFunc_LoadingSplash::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void UNxFunc_LoadingSplash::PreloadBackgroundImages()
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

void UNxFunc_LoadingSplash::RemovePreloadedBackgroundImages()
{
	if (FNxLoadingSplashModule::IsAvailable())
	{
		FNxLoadingSplashModule& LoadingScreenModule = FNxLoadingSplashModule::Get();
		LoadingScreenModule.RemoveAllBackgroundImages();
	}
}

