
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FNxLoadingSplashScreen;

class  FNxLoadingSplashModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	 
	/**
	 * 모듈 DLL이 로드되고 모듈 개체가 만들어진 직후에 호출됩니다.
	 */
	virtual void StartupModule() override;
	
	/**
	 * 모듈이 언로드되기 전, 즉 모듈 객체가 파기되기 직전에 호출됩니다.
	 */
	virtual void ShutdownModule() override;

	/**
	 * 이 모듈이 게임 플레이 코드를 호스팅하는 경우 true 를 반환합니다.
	 * @return "gameplay modules"의 경우 True, 엔진 코드 모듈, 플러그인 등의 경우 false.
	 */
	virtual bool IsGameModule() const override;

	/**
	 * 이 모듈의 인터페이스에 싱글 톤과 같은 액세스 지원, 편의성 제공
	 * Shutdown 단계 동안 호출에 유의하십시오. 모듈이 이미 언로드되었을 수 있습니다.
	 * @return 싱글 톤 인스턴스를 반환하고 필요에 따라 주문형으로 모듈을 로드합니다.	 
	 */
	static inline FNxLoadingSplashModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FNxLoadingSplashModule>("NxLoadingSplash");
	}

	/**
	 * 이 모듈이 로드되어 준비가 되어 있는지 확인합니다. Get() 호출은 IsAvailable()이 true를 반환하는 경우에만 유효합니다.
	 * @return 모듈이 로드되어 사용할 준비가 되면 True
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("NxLoadingSplash");
	}

	TArray<UTexture2D*> GetBackgroundImages();

	/**
	 * 'bPreloadBackgroundImages' 옵션이 Enable 설정되어 있는지 확인.
	 */
	bool IsPreloadBackgroundImagesEnabled();

	/**
	 * 시작시로드 화면이 표시됩니까?
	 */
	bool IsStartupLoadingScreen() { return bIsStartupLoadingScreen; }

	/**
	 * 설정된 모든 배경 이미지들을 배열로 로드합니다.
	 */
	void LoadBackgroundImages();

	/**
	 * 배열에서 모든 배경 이미지를 삭제합니다.
	 */
	void RemoveAllBackgroundImages();

private:
	/**
	 * 로딩 화면 콜백, 로딩 화면을 이미 명시적으로 설정한 경우에는 호출되지 않습니다.
	 */
	void PreSetupLoadingScreen();

	/**
	 * 로딩 화면 설정값 세팅
	 */
	void SetupLoadingScreen(const FNxLoadingSplashScreen& LoadingScreenSettings);

	/**
	 * 무비 목록을 섞습니다.
	 */
	void ShuffleMovies(TArray<FString>& MoviesList);

private:
	// Startup 배경 이미지 배열
	UPROPERTY()
	TArray<class UTexture2D*> StartupBackgroundImages;
	
	// Default 배경 이미지 배열
	UPROPERTY()
	TArray<class UTexture2D*> DefaultBackgroundImages;

	// 시작시 로딩 스크린 사용 여부
	bool bIsStartupLoadingScreen = false;
};


// 참고:
/************************************************************************************
 * Copyright (C) 2020 Truong Bui.													*
 * Website:		https://github.com/truong-bui/AsyncLoadingScreen					*
 * SupportURL:	https://github.com/truong-bui/AsyncLoadingScreen/issues				*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 ************************************************************************************/


