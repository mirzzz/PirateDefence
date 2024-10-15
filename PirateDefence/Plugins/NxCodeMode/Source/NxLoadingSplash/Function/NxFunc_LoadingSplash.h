
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NxFunc_LoadingSplash.generated.h"
 
/**
 * Splash Loading Function Library
 */
UCLASS()
class NXLOADINGSPLASH_API UNxFunc_LoadingSplash : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
private:
	static int32 DisplayBackgroundIndex;
	static int32 DisplayTipTextIndex;
	static int32 DisplayMovieIndex;	
	static bool  bShowLoadingScreen;

public:
	
	/**
	 * 로딩 화면에 어떤 배경을 표시할지를 인덱스로 설정합니다. 
	 * 이 기능을 사용하려면 배경 설정의 SetDisplayBackgroundManually 옵션을 true로 설정해야 합니다.
	 * @param BackgroundIndex 배경 설정의 "Images" 배열에서 배경의 유효한 인덱스. 색인이 유효하지 않으면 대신 무작위 배경이 표시됩니다.	 
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void SetDisplayBackgroundIndex(int32 BackgroundIndex);

	/**
	 * 로딩 화면에 어떤 텍스트를 표시할지를 인덱스로 설정합니다. 
	 * 이 기능을 사용하려면 팁 위젯 설정의 SetDisplayTipTextManually 옵션을 true로 설정해야 합니다.
	 * @param TipTextIndex 팁 위젯 설정의 "TipText" 배열에서 텍스트의 유효한 인덱스. 색인이 유효하지 않으면 대신 임의의 텍스트가 표시됩니다.
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void SetDisplayTipTextIndex(int32 TipTextIndex);

	/**
	 * 로딩 화면에 어떤 동영상을 표시할지를 인덱스로 설정합니다. 
	 * 이 기능을 사용하려면 SetDisplayMovieIndexManually 옵션을 true로 설정해야 합니다.
	 * @param MovieIndex "MoviePaths" 배열의 동영상에 유효한 인덱스입니다.
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void SetDisplayMovieIndex(int32 MovieIndex);


	/**
	 * 레벨의 로드 화면을 활성화/비활성화 설정
	 * @param bIsEnableLoadingScreen 다음 레벨로드 화면을 활성화해야합니까?
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void SetEnableLoadingScreen(bool bIsEnableLoadingScreen);


	/**
	 * 레벨의 로드 화면을 활성화/비활성화 상태 반환.
	 **/
	UFUNCTION(BlueprintPure, Category = LoadingSplash)
	static inline bool GetIsEnableLoadingScreen() { return bShowLoadingScreen; }

	/**
	 * 로드 화면을 정지합니다. 이 기능을 사용하려면 bAllowEngineTick 옵션을 활성화해야 합니다.
	 * BeginPlay 이벤트에서 이 함수를 호출하여 로드 화면을 중지합니다 (Delay 노드에서 작동함).	 
	 *
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void StopLoadingScreen();

	static inline int32 GetDisplayBackgroundIndex() { return DisplayBackgroundIndex; }
	static inline int32 GetDisplayTipTextIndex() { return DisplayTipTextIndex; }
	static inline int32 GetDisplayMovieIndex() { return DisplayMovieIndex; }

	/**
	 * 모든 배경 이미지를 메모리에 로드합니다.
	 *
	 * 다음 경우에만 호출 필요:
	 * "RemovePreloadedBackgroundImages"를 호출하기 전, 'bPreloadBackgroundImages' 옵션이 체크된 상태이면,
	 * 배경이미지들이 아직 메모리에 남아있기 때문에, "RemovePreloadedBackgroundImages" 함수가 호출되지 않는 이상, 이함수를 사용할 필요는 없습니다
	 * 참고: [OpenLevel]노드를 호출하기 전에 이 함수를 호출해야 배경이미지가 정상 작동 합니다.
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void PreloadBackgroundImages();

	/**
	 * 미리 로드된 배경 이미지를 모두 삭제합니다.
	 * 
	 * 이 기능은 다음과 같은 경우에만 사용해야 합니다.
	 * [bPreloadBackgroundImages]에 체크가 되어 있을때.
	 * 
	 * 이 함수 호출 후에는, 'PreloadBackgroundImages'를 호출해서 모든 배경 이미지를 수동으로 다시 로드해야 합니다.
	 **/
	UFUNCTION(BlueprintCallable, Category = LoadingSplash)
	static void RemovePreloadedBackgroundImages();
};
