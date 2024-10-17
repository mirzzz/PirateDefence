
#include "NxLoadingSplashModule.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "MoviePlayer.h"

#include "NxLoadingSplashSettings.h"
#include "SCenterLayout.h"
#include "SClassicLayout.h"
#include "SLetterboxLayout.h"
#include "SSidebarLayout.h"
#include "SDualSidebarLayout.h"
#include "NxLoadingSplashLibrary.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxLoadingSplashModule"

void FNxLoadingSplashModule::StartupModule()
{	
	NxPrintTxt(LogLoading, TEXT("[----------------------------------------------------- NxLoadingSplash Module Startup]"));

	// 모듈이 메모리에 로드된 후에 실행됩니다. 정확한 타이밍은 모듈별 .uplugin 파일에 지정되어 있습니다.
	if (!IsRunningDedicatedServer() && FSlateApplication::IsInitialized())
	{
		const UNxLoadingSplashSettings* settings = GetDefault<UNxLoadingSplashSettings>();
		const bool bUse = settings->bUseAsyncLoading;
		if (bUse)
		{
			// PreloadBackgroundImages 옵션을 선택하면 모든 배경 이미지를 메모리에 로드합니다.
			if (settings->bPreloadBackgroundImages)
			{
				LoadBackgroundImages();
			}

			if (IsMoviePlayerEnabled())
			{
				GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, &FNxLoadingSplashModule::PreSetupLoadingScreen);
			}

			// 시작 화면을 준비합니다. 
			//	로딩 화면을 이미 명시적으로 설정한 경우 PreSetupLoadingScreen 콜백이 호출되지 않습니다.
			bIsStartupLoadingScreen = true;
			SetupLoadingScreen(settings->StartupLoadingSettings);
		}
	}	
}

void FNxLoadingSplashModule::ShutdownModule()
{
	// 모듈을 정리하기 위해 종료 중에 호출될 수 있습니다.
	// 동적 재로딩을 지원하는 모듈의 경우 모듈을 언로드하기 전에 이 함수를 호출 합니다.
	if (!IsRunningDedicatedServer())
	{
		// GetDefault<UVxAsyncLoadingSettings>() 이미 해당 객체는 제거되었음. 호출시 nullptr;

		// E_ActiveLoadingType::ALT_Async			
		GetMoviePlayer()->OnPrepareLoadingScreen().RemoveAll(this);
		
		// E_ActiveLoadingType::ALT_Widget
	}
		
	NxPrintTxt(LogLoading, TEXT("[----------------------------------------------------- NxLoadingSplash Module Shutdown]"));
}

bool FNxLoadingSplashModule::IsGameModule() const
{
	return true;
}

TArray<UTexture2D*> FNxLoadingSplashModule::GetBackgroundImages()
{
	return bIsStartupLoadingScreen ? StartupBackgroundImages : DefaultBackgroundImages;
}

void FNxLoadingSplashModule::PreSetupLoadingScreen()
{	
	NxPrintDisplay(LogLoading, TEXT("PreSetupLoadingScreen"));

	const UNxLoadingSplashSettings* settings = GetDefault<UNxLoadingSplashSettings>();
	const bool bUse = settings->bUseAsyncLoading;
	if (bUse)
	{
		const bool bIsEnableLoadingScreen = UNxLoadingSplashLibrary::GetIsEnableLoadingScreen();
		if (bIsEnableLoadingScreen)
		{
			bIsStartupLoadingScreen = false;
			SetupLoadingScreen(settings->DefaultLoadingSettings);
		}	
	}
}

void FNxLoadingSplashModule::SetupLoadingScreen(const FNxLoadingSplashScreen& LoadingScreenSettings)
{
	NxPrintDisplay(LogLoading, TEXT("SetupLoadingScreen"));

	TArray<FString> moviesList = LoadingScreenSettings.MoviePaths;

	// movies 목록 셔플
	if (LoadingScreenSettings.bShuffle == true)
	{
		ShuffleMovies(moviesList);
	}
		
	if (LoadingScreenSettings.bSetDisplayMovieIndexManually == true)
	{
		moviesList.Empty();

		// 유효한 경우 특정 movie를 표시하고, 그렇지 않으면 원본 movie 목록을 표시합니다.
		if (LoadingScreenSettings.MoviePaths.IsValidIndex(UNxLoadingSplashLibrary::GetDisplayMovieIndex()))
		{
			moviesList.Add(LoadingScreenSettings.MoviePaths[UNxLoadingSplashLibrary::GetDisplayMovieIndex()]);
		}
		else
		{
			moviesList = LoadingScreenSettings.MoviePaths;
		}
	}

	FLoadingScreenAttributes loadingScreen;
	loadingScreen.MinimumLoadingScreenDisplayTime	= LoadingScreenSettings.MinimumLoadingScreenDisplayTime;
	loadingScreen.bAutoCompleteWhenLoadingCompletes = LoadingScreenSettings.bAutoCompleteWhenLoadingCompletes;
	loadingScreen.bMoviesAreSkippable				= LoadingScreenSettings.bMoviesAreSkippable;
	loadingScreen.bWaitForManualStop				= LoadingScreenSettings.bWaitForManualStop;
	loadingScreen.bAllowInEarlyStartup				= LoadingScreenSettings.bAllowInEarlyStartup;
	loadingScreen.bAllowEngineTick					= LoadingScreenSettings.bAllowEngineTick;
	loadingScreen.MoviePaths						= moviesList;
	loadingScreen.PlaybackType						= LoadingScreenSettings.PlaybackType;

	if (LoadingScreenSettings.bShowWidgetOverlay)
	{
		const UNxLoadingSplashSettings* settings = GetDefault<UNxLoadingSplashSettings>();

		switch (LoadingScreenSettings.Layout)
		{
		case E_LoadingSplashLayout::LSL_Classic:
			loadingScreen.WidgetLoadingScreen = SNew(SClassicLayout, LoadingScreenSettings, settings->Classic);
			break;
		case E_LoadingSplashLayout::LSL_Center:
			loadingScreen.WidgetLoadingScreen = SNew(SCenterLayout, LoadingScreenSettings, settings->Center);
			break;
		case E_LoadingSplashLayout::LSL_Letterbox:
			loadingScreen.WidgetLoadingScreen = SNew(SLetterboxLayout, LoadingScreenSettings, settings->Letterbox);
			break;
		case E_LoadingSplashLayout::LSL_Sidebar:
			loadingScreen.WidgetLoadingScreen = SNew(SSidebarLayout, LoadingScreenSettings, settings->Sidebar);
			break;
		case E_LoadingSplashLayout::LSL_DualSidebar:
			loadingScreen.WidgetLoadingScreen = SNew(SDualSidebarLayout, LoadingScreenSettings, settings->DualSidebar);
			break;
		}
	}

	GetMoviePlayer()->SetupLoadingScreen(loadingScreen);
}

void FNxLoadingSplashModule::ShuffleMovies(TArray<FString>& MoviesList)
{
	if (MoviesList.Num() > 0)
	{
		int32 lastIndex = MoviesList.Num() - 1;
		for (int32 i = 0; i <= lastIndex; ++i)
		{
			int32 index = FMath::RandRange(i, lastIndex);
			if (i != index)
			{
				MoviesList.Swap(i, index);
			}
		}
	}
}

void FNxLoadingSplashModule::LoadBackgroundImages()
{
	// 배열 초기화
	RemoveAllBackgroundImages();

	const UNxLoadingSplashSettings* settings = GetDefault<UNxLoadingSplashSettings>();
	const bool bUse = settings->bUseAsyncLoading;

	// startup/defualt background images 로딩
	if(bUse)
	{
		for (auto& Image : settings->StartupLoadingSettings.Background.Images)
		{
			UTexture2D* LoadedImage = Cast<UTexture2D>(Image.TryLoad());
			if (LoadedImage)
			{
				StartupBackgroundImages.Add(LoadedImage);
			}
		}

		for (auto& Image : settings->DefaultLoadingSettings.Background.Images)
		{
			UTexture2D* LoadedImage = Cast<UTexture2D>(Image.TryLoad());
			if (LoadedImage)
			{
				DefaultBackgroundImages.Add(LoadedImage);
			}
		}
	}
}

void FNxLoadingSplashModule::RemoveAllBackgroundImages()
{
	StartupBackgroundImages.Empty();
	DefaultBackgroundImages.Empty();
}

bool FNxLoadingSplashModule::IsPreloadBackgroundImagesEnabled()
{	
	return GetDefault<UNxLoadingSplashSettings>()->bPreloadBackgroundImages;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNxLoadingSplashModule, NxLoadingSplash)