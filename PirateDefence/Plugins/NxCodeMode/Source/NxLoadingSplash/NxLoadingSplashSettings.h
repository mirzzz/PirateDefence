
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MoviePlayer.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Styling/SlateBrush.h"
#include "Framework/Text/TextLayout.h"
#include "NxLoadingSplashSettings.generated.h"

 // 비동기 로딩 레이아웃 종류
UENUM(BlueprintType)
enum class E_LoadingSplashLayout : uint8
{
	/**
	 * 클래식은 간단하고 범용적인 레이아웃으로 많은 디자인에 잘 맞습니다.
	 * 가져오기 위젯과 팁 위젯은 하단 또는 상단에 위치할 수 있습니다.
	 */
	LSL_Classic UMETA(DisplayName = "Classic"),
	/** 
	 * 로딩 위젯은 화면 중앙에 있으며 힌트 위젯은 하단 또는 상단에 있습니다.
	 * 가져오기 아이콘이 기본 디자인인 경우 중앙 레이아웃이 적합합니다.
	 */
	LSL_Center UMETA(DisplayName = "Center"),
	/**
	 * 레터박스 레이아웃에는 화면 상단과 하단에 두 개의 테두리가 있습니다. 
	 * 로딩 위젯은 화면 상단에 있고 팁은 화면 하단에 있을 수 있으며, 그 반대도 가능합니다.
	 */
	 LSL_Letterbox UMETA(DisplayName = "Letterbox"),
	/**
	 * 사이드바 레이아웃에는 화면 왼쪽이나 오른쪽에 세로 테두리가 있습니다. 
	 * 사이드바는 팁 위젯의 높이로 인해 스토리텔링, 긴 문단에 적합합니다.
	 */
	 LSL_Sidebar UMETA(DisplayName = "Sidebar"),

	/**
	 * 사이드바 레이아웃과 유사하지만 듀얼 사이드바 레이아웃에는 화면 왼쪽과 오른쪽에 두 개의 수직 테두리가 있습니다.
	 * 듀얼 사이드바 레이아웃은 팁 위젯의 높이로 인해 스토리텔링, 긴 문단에 적합합니다
	 */
	 LSL_DualSidebar UMETA(DisplayName = "Dual Sidebar")
};


// 로딩 아이콘 유형
UENUM(BlueprintType)
enum class E_LoadingIconType : uint8
{		
	/** SThrobber widget */
	LIT_Throbber			UMETA(DisplayName = "Throbber"),
	/** SCircularThrobber widget */
	LIT_CircularThrobber	UMETA(DisplayName = "Circular Throbber"),
	/** Animated images */
	LIT_ImageSequence		UMETA(DisplayName = "Image Sequence")
};

// 위젯 정렬 유형 
UENUM(BlueprintType)
enum class E_LoadingWidgetType : uint8
{
	/** Horizontal alignment */
	LWT_Horizontal	UMETA(DisplayName = "Horizontal"),
	/** Vertical alignment */
	LWT_Vertical	UMETA(DisplayName = "Vertical"),
};

// 위젯 정렬
USTRUCT(BlueprintType)
struct FNxWidgetAlignment
{
	GENERATED_BODY()
	// 위젯의 수평 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment Setting")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Center;

	// 위젯의 수직 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment Setting")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Center;
};

// 텍스트 모양 설정 
USTRUCT(BlueprintType)
struct FNxTextAppearance
{
	GENERATED_BODY()

	// 텍스트 색상 및 불투명도
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	// 텍스트를 렌더링할 글꼴
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateFontInfo Font;

	// 픽셀 단위의 그림자 오프셋
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FVector2D ShadowOffset = FVector2D::ZeroVector;

	// 그림자 색상 및 불투명도
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FLinearColor ShadowColorAndOpacity = FLinearColor::White;

	// 텍스트를 여백에 맞춰 정렬하는 방법
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	TEnumAsByte <ETextJustify::Type> Justification = ETextJustify::Left;
};

USTRUCT(BlueprintType)
struct FNxThrobberSettings
{
	GENERATED_BODY()

	// 조각 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"))
	int32 NumberOfPieces = 3;

	// 조각이 수평으로 애니메이션되어야 합니까?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateHorizontally = true;

	// 조각이 수직으로 애니메이션되어야 합니까?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateVertically = true;

	// 조각의 불투명도에 애니메이션을 적용해야 합니까?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	bool bAnimateOpacity = true;

	// Throbber의 각 세그먼트에 사용할 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush Image;
};

USTRUCT(BlueprintType)
struct FNxCircularThrobberSettings
{
	GENERATED_BODY()

	// 조각 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (ClampMin = "1", ClampMax = "25", UIMin = "1", UIMax = "25"))
	int32 NumberOfPieces = 6; 

	// 한 바퀴를 도는 데 걸리는 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (ClampMin = "0", UIMin = "0"))
	float Period = 0.75f;

	// 원의 반경. Throbber가 Canvas Panel의 하위인 경우 반경을 설정하려면 'Size to Content' 옵션을 활성화해야 합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
	float Radius = 64.0f;

	// Throbber의 각 세그먼트에 사용할 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance)
	FSlateBrush Image;
};

USTRUCT(BlueprintType)
struct FNxImageSequenceSettings
{
	GENERATED_BODY()

	// 로딩 아이콘 애니메이션을 위한 이미지 배열
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TArray<UTexture2D*> Images;

	// 이미지의 크기
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	// 이미지를 업데이트하는 시간(초) 값이 작을수록 애니메이션 속도가 빨라집니다. 값이 0이면 프레임마다 이미지가 업데이트됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (UIMax = 1.00, UIMin = 0.00, ClampMin = "0", ClampMax = "1"))
	float Interval = 0.05f;

	// 이미지 시퀀스를 역방향으로 재생합니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	bool bPlayReverse = false;
};

/**
 * 위젯 로딩 화면의 배경 위젯
 */
USTRUCT(BlueprintType)
struct NXLOADINGSPLASH_API FNxBackgroundSettings
{
	GENERATED_BODY()

	// Movie의 상단에 로딩 이미지를 무작위로 표시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TArray<FSoftObjectPath> Images;

	// 이미지에 적용할 크기 조정 유형입니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	TEnumAsByte<EStretch::Type> ImageStretch = EStretch::ScaleToFit;

	// 테두리와 포함된 이미지 사이의 패딩 영역입니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	FMargin Padding;

	// 정의된 이미지가 있는 경우 테두리의 배경색입니다. padding = 0 이면 테두리 색상이 표시되지 않습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	FLinearColor BackgroundColor = FLinearColor::Black;

	// true인 경우 "SetDisplayBackgroundIndex" 함수를 호출하여 로딩 화면에 표시할 배경 인덱스를 수동으로 설정해야 합니다.
	// 새 레벨을 열기 전에 블루프린트에서 설정한 인덱스가 유효하지 않으면 "이미지" 배열에 임의의 배경이 표시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	bool bSetDisplayBackgroundManually = false;
};

/**
 * 로딩 위젯 설정 
 */
USTRUCT(BlueprintType)
struct NXLOADINGSPLASH_API FNxLoadingWidgetSettings
{
	GENERATED_BODY()

	FNxLoadingWidgetSettings();

	// 로딩 아이콘 유형
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	E_LoadingIconType LoadingIconType = E_LoadingIconType::LIT_CircularThrobber;
	
	// 위젯 유형 로딩
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	E_LoadingWidgetType LoadingWidgetType = E_LoadingWidgetType::LWT_Horizontal;

	// 로딩 아이콘의 렌더 변환 변환입니다
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformTranslation = FVector2D(0.0f, 0.0f);

	// 로딩 아이콘의 렌더 변환 스케일, 음수 값은 아이콘을 뒤집습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformScale = FVector2D(1.0f, 1.0f);

	// 로딩 아이콘의 렌더링 변환 피벗(정규화된 로컬 공간).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D TransformPivot = FVector2D(0.5f, 0.5f);

	// 애니메이션 아이콘 옆에 표시되는 텍스트
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FText LoadingText;

	// 로딩 아이콘 오른쪽에 로딩 텍스트가 있나요? Loading Widget Type = Horizontal 을 선택하지 않으면 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (EditCondition = "LoadingWidgetType == E_LoadingWidgetType::LWT_Horizontal", EditConditionHides))
	bool bLoadingTextRightPosition = true;

	// 로딩 아이콘 상단에 로딩 텍스트가 있나요? Loading Widget Type = Vertical를 선택하지 않으면 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (EditCondition = "LoadingWidgetType == E_LoadingWidgetType::LWT_Vertical", EditConditionHides))
	bool bLoadingTextTopPosition = true;

	// 텍스트 모양 설정 로드
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
	FNxTextAppearance Appearance;

	// 트로버 설정. 'Throbber' 아이콘 유형을 선택하지 않으면 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FNxThrobberSettings ThrobberSettings;

	// Circular Throbber 설정. 'Circular Throbber' 아이콘 유형을 선택하지 않으면 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FNxCircularThrobberSettings CircularThrobberSettings;

	// 이미지 시퀀스 설정. '이미지 시퀀스' 아이콘 유형을 선택하지 않으면 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FNxImageSequenceSettings ImageSequenceSettings;

	// 로딩 텍스트의 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	FNxWidgetAlignment TextAlignment;

	// 로딩 아이콘의 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	FNxWidgetAlignment LoadingIconAlignment;

	// 로딩 텍스트와 로딩 아이콘 사이의 빈 공간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	float Space = 1.0f;

	// 레벨 로딩이 완료되면 로딩 위젯 숨기기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	bool bHideLoadingWidgetWhenCompletes = false;
};


/**
 * 팁 텍스트 설정
 */
USTRUCT(BlueprintType)
struct NXLOADINGSPLASH_API FNxTipSettings
{
	GENERATED_BODY()

	// 로딩 화면에 팁 텍스트가 무작위로 표시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings", meta = (MultiLine = true))
	TArray<FText> TipText;

	// 팁 텍스트 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
	FNxTextAppearance Appearance;

	// 다음 줄로 넘어가기 전 팁의 크기
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
	float TipWrapAt = 0.0f;

	// true인 경우 새 레벨을 열기 전에 블루프린트에서 "SetDisplayTipTextIndex" 함수를 호출하여
	// 로딩 화면에 표시할 TipText 인덱스를 수동으로 설정해야 합니다. 설정한 인덱스가 유효하지 않으면 "TipText" 배열에 임의의 팁이 표시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
	bool bSetDisplayTipTextManually = false;
};

 // 로딩이 완료되면 표시되는 텍스트입니다. "bShowLoadingCompletedText"=true 로 설정하지 않으면 무시 됩니다.
USTRUCT(BlueprintType)
struct NXLOADINGSPLASH_API FNxLoadingCompleteTextSettings
{
	GENERATED_BODY()

	// FVxLoadingCompleteTextSettings();

	// 레벨 로딩이 완료되면 표시되는 텍스트
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FText LoadingCompleteText;

	// 텍스트 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FNxTextAppearance Appearance;
	
	// 텍스트 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
	FNxWidgetAlignment Alignment;

	// 텍스트 패딩
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	FMargin Padding;

	// 텍스트에 애니메이션을 적용하시겠습니까?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings")
	bool bFadeInOutAnim = true;

	// 애니메이션 속도
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Complete Text Settings", meta = (UIMax = 10.00, UIMin = 0.00, ClampMin = "0", ClampMax = "10"))
	float AnimationSpeed = 1.0f;
};


 // 로딩 화면 설정
USTRUCT(BlueprintType)
struct NXLOADINGSPLASH_API FNxLoadingSplashScreen
{
	GENERATED_BODY()	

	// 로딩 화면이 열려야 하는 최소 시간입니다. 최소 시간이 없으면 -1입니다. (-1로 설정하는 것이 좋습니다)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	float MinimumLoadingScreenDisplayTime = -1;
	
	// true인 경우 로딩이 완료되자마자 로딩 화면이 사라집니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAutoCompleteWhenLoadingCompletes = true;

	// true인 경우 로딩이 완료되는 동안 로딩 화면을 클릭하여 Movie를 건너뛸 수 있습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bMoviesAreSkippable = true;

	// true인 경우, Stop 이 호출될 때까지 동영상 재생이 계속됩니다.
	// 참고: 
	//	"Minimum Loading Screen Display Time" = -1로 설정하면 플레이어가 아무 키나 눌러 로딩 화면을 중지할 수 있습니다.
	//	"Minimum Loading Screen Display Time" >= 0인 경우 GameInstance, GameMode 또는 PlayerController 블루프린트의 
	//	  BeginPlay 이벤트에서 "StopLoadingScreen"을 호출하여 로딩 화면을 중지해야 합니다 ("bAllowEngineTick"이 true여야 함).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bWaitForManualStop = false;

	// 실제 로딩 화면이라면 어떤 종류의 uobject도 가질 수 없으며 엔진 기능을 전혀 사용할 수 없습니다. 
	// 이를 지원하는 플랫폼에서는 결과적으로 Movie가 매우 일찍 시작됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowInEarlyStartup = false;

	// true인 경우 게임 스레드가 동영상 로드가 완료되기를 기다리는 동안 엔진 틱을 호출합니다. 
	// 이는 시작 후 로드 화면에서만 작동하며 잠재적으로 안전하지 않습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowEngineTick = false;

	// Movie의 재생/반복 등을 해야 할까요? 
	// 참고: 
	//	재생 유형이 MT_LoopLast이면 마지막 동영상이 히트될 때 bAutoCompleteWhenLoadingCompletes가 켜집니다
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	TEnumAsByte<EMoviePlaybackType> PlaybackType = EMoviePlaybackType::MT_Normal;

	// 모든 Movie 파일은 Content/Movies/ 디렉토리에 있어야 합니다. 권장 형식: MPEG-4 동영상(mp4). 확장자 없이 파일 경로/이름을 입력하세요.
	// 예를 들어 'Content/Movies' 폴더에 Movie 이름이 my_movie.mp4 인 경우 입력 필드에 my_movie를 입력하세요.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	TArray<FString> MoviePaths;
	
	// true인 경우 재생하기 전에 Movie 목록을 섞습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bShuffle = false;

	// true인 경우 "Shuffle" 옵션은 무시되며, 새 레벨을 열기 전에 블루프린트에서 "SetDisplayMovieIndex" 함수를 호출하여
	// 로딩 화면에 표시할 영화 인덱스를 수동으로 설정해야 합니다.	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bSetDisplayMovieIndexManually = false;

	// 로딩 화면 위젯(배경/팁/로딩 위젯)을 표시해야 할까요? 
	// - 일반적으로 Movie만 보여주고 싶다면 false로 설정하는 것이 좋습니다.	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	bool bShowWidgetOverlay = true;		

	// true인 경우 레벨 로딩이 완료되면 텍스트를 표시합니다. "bShowWidgetOverlay" = false 를 선택한 경우 무시됩니다.
	//	참고: 이 옵션을 제대로 활성화하려면 "Wait For Manual Stop" = true, "Minimum Loading Screen Display Time" = -1로 설정해야 합니다.
	//	      또한, 이를 통해 플레이어는 아무 버튼이나 눌러 로딩 화면을 중지할 수 있습니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	bool bShowLoadingCompleteText = false;

	// 로딩이 완료되면 표시되는 텍스트입니다. "Show Loading Complete Text" = false로 설정한 경우 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FNxLoadingCompleteTextSettings LoadingCompleteTextSettings;

	// 로딩 화면의 배경 위젯입니다. "bShowWidgetOverlay = false"를 선택한 경우 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FNxBackgroundSettings Background;	
	
	// 로딩 화면에 대한 팁 위젯입니다. "bShowWidgetOverlay = false"를 선택한 경우 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FNxTipSettings TipWidget;

	// 로딩 화면용 로딩 위젯입니다. "bShowWidgetOverlay = false"를 선택한 경우 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FNxLoadingWidgetSettings LoadingWidget;

	// 비동기 로딩 화면 레이아웃을 선택하세요. "Show Widget Overlay = false"를 선택한 경우 무시됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	E_LoadingSplashLayout Layout = E_LoadingSplashLayout::LSL_Classic;
};

/** Classic Layout settings*/
USTRUCT(BlueprintType)
struct FNxClassicLayoutSettings
{
	GENERATED_BODY()

	// 로딩과 팁 위젯이 포함된 테두리는 하단에 있나요, 아니면 상단에 있나요?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	bool bIsWidgetAtBottom = true;

	// 팁 왼쪽에 위젯 로딩이 있나요?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	bool bIsLoadingWidgetAtLeft = true;	

	// 로딩 위젯과 팁 사이의 빈 공간
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	float Space = 1.0f;

	// Tip(팁) 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classic Layout")
	FNxWidgetAlignment TipAlignment;

	// border background(테두리 배경)의 가로 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Center Layout")
	TEnumAsByte<EHorizontalAlignment> BorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	// 테두리와 테두리가 포함된 위젯 사이의 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	FMargin BorderPadding;

	// 테두리 위젯의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Classic Layout")
	FSlateBrush BorderBackground;	
};

/** Center Layout settings*/
USTRUCT(BlueprintType)
struct FNxCenterLayoutSettings
{
	GENERATED_BODY()

	// 팁은 아래쪽에 있나요, 아니면 위쪽에 있나요?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	bool bIsTipAtBottom = true;	

	// Tip(팁) 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Center Layout")
	FNxWidgetAlignment TipAlignment;

	// border background(테두리 배경)의 가로 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Center Layout")
	TEnumAsByte<EHorizontalAlignment> BorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	// 하단 또는 상단 위치에 있는 팁에 따라 화면 하단 또는 상단으로 오프셋됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	float BorderVerticalOffset = 0.0f;

	// 테두리와 테두리에 포함된 팁 사이의 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	FMargin BorderPadding;

	// 팁 영역의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Center Layout")
	FSlateBrush BorderBackground;
};

/** Letterbox Layout settings*/
USTRUCT(BlueprintType)
struct FNxLetterboxLayoutSettings
{
	GENERATED_BODY()

	// 로딩 위젯이 하단에 있나요, 아니면 상단에 있나요?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	bool bIsLoadingWidgetAtTop = true;

	// Tip(팁) 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Letterbox Layout")
	FNxWidgetAlignment TipAlignment;

	// 로딩 위젯의 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Letterbox Layout")
	FNxWidgetAlignment LoadingWidgetAlignment;

	// 위쪽 테두리의 가로 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Letterbox Layout")
	TEnumAsByte<EHorizontalAlignment> TopBorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	// 아래쪽 테두리의 가로 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Letterbox Layout")
	TEnumAsByte<EHorizontalAlignment> BottomBorderHorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	// 테두리와 테두리가 포함된 위젯 사이의 상단 패딩 영역.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FMargin TopBorderPadding;

	// 테두리와 테두리가 포함된 위젯 사이의 하단 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FMargin BottomBorderPadding;

	// 위쪽 테두리의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FSlateBrush TopBorderBackground;

	// 하단 테두리의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Letterbox Layout")
	FSlateBrush BottomBorderBackground;
};

/** Sidebar Layout settings*/
USTRUCT(BlueprintType)
struct FNxSidebarLayoutSettings
{
	GENERATED_BODY()

	// 로딩 및 팁 위젯이 포함된 테두리가 오른쪽에 있습니까, 아니면 왼쪽에 있습니까?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	bool bIsWidgetAtRight = true;

	// 팁 상단에 위젯이 로딩되고 있나요?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	bool bIsLoadingWidgetAtTop = true;

	// 로딩 위젯과 팁 사이의 빈 공간
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	float Space = 1.0f;	

	// 로딩/팁 위젯이 포함된 수직 상자의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Center;

	// 로딩 위젯의 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sidebar Layout")
	FNxWidgetAlignment LoadingWidgetAlignment;

	// 팁 정렬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sidebar Layout")
	FNxWidgetAlignment TipAlignment;

	// 모든 위젯을 포함하는 테두리 배경의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> BorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	// 왼쪽 또는 오른쪽 위치에 있는 로딩 및 팁 위젯이 포함된 테두리에 따라 화면 왼쪽 또는 오른쪽으로 오프셋됩니다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	float BorderHorizontalOffset = 0.0f;

	// 테두리와 포함된 위젯 사이의 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FMargin BorderPadding;

	// 테두리 위젯의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sidebar Layout")
	FSlateBrush BorderBackground;
};

/** Dual Sidebar Layout settings*/
USTRUCT(BlueprintType)
struct FNxDualSidebarLayoutSettings
{
	GENERATED_BODY()

	// 오른쪽 또는 왼쪽 테두리에 위젯을 로드하고 있습니까?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	bool bIsLoadingWidgetAtRight = true;

	// 왼쪽 위젯의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> LeftVerticalAlignment = EVerticalAlignment::VAlign_Center;

	// 오른쪽 위젯의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> RightVerticalAlignment = EVerticalAlignment::VAlign_Center;

	// 모든 위젯을 포함하는 왼쪽 테두리 배경의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> LeftBorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	// 모든 위젯을 포함하는 오른쪽 테두리 배경의 수직 정렬
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	TEnumAsByte<EVerticalAlignment> RightBorderVerticalAlignment = EVerticalAlignment::VAlign_Fill;

	// 왼쪽 테두리와 여기에 포함된 위젯 사이의 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FMargin LeftBorderPadding;

	// 오른쪽 테두리와 여기에 포함된 위젯 사이의 패딩 영역
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FMargin RightBorderPadding;

	// 왼쪽 테두리 위젯의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FSlateBrush LeftBorderBackground;

	// 오른쪽 테두리 위젯의 배경 모양 설정
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dual Sidebar Layout")
	FSlateBrush RightBorderBackground;
};


/**
 * LoadingSplash Screen Settings 
 */
UCLASS(Config = Plugin, DefaultConfig, meta = (DisplayName = "LoadingSettings"))
class NXLOADINGSPLASH_API UNxLoadingSplashSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UNxLoadingSplashSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// 프로젝트 Category 설정
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { /*return FApp::GetProjectName();*/ return FName("Nx"); }
	virtual FName GetSectionName() const override { return FName("NxLoadingSplash"); }

#if WITH_EDITOR
	// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:

	// 시작 및 레벨 로딩 시 LoadingScreen 노출 여부
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash")
	bool bUseAsyncLoading = false;
		
	// true인 경우 게임 시작 시 모든 배경 이미지를 로드합니다.
	//	- 배경 이미지가 너무 늦게 로드되어 잘못된 이미지 크기 조정 발생하는  문제에 대한 해결 방법입니다. 
	//	- 이런 현상은 독립형 또는 실행 모드에서만 발생하며, 패키지된 게임이 제대로 작동할 것입니다.
	//	- 개발할 때 이런 현상이 발생하지 않으면 이 옵션을 활성화 하지 마십시오. 배경 이미지가 항상 메모리에 유지되어 메모리 리소스를 소모하기 때문입니다.
	//	- 또한, 블루프린트 함수 "RemovePreloadedBackgroundImages"를 호출하여 미리 로드된 모든 배경 이미지를 수동으로 제거할 수 있으나,
	//	- 필요할 경우 "PreloadBackgroundImages"를 호출하여 모든 배경 이미지를 다시 로드해야 합니다.
	//	  참고: "OpenLevel" 전에 "PreloadBackgroundImages"를 호출하세요.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash")
	bool bPreloadBackgroundImages = false;
		
	// 게임을 처음 실행했을 때의 시작 로딩 화면입니다. 여기에서 스튜디오 로고 동영상을 설정하세요.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash", meta = (EditCondition = "bUseAsyncLoading==True"))
	FNxLoadingSplashScreen StartupLoadingSettings;
		
	// 새로운 레벨을 열 때마다 나타나는 기본 로딩 화면-Static 버전
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash", meta = (EditCondition = "bUseAsyncLoading==True"))
	FNxLoadingSplashScreen DefaultLoadingSettings;

	// 클래식 레이아웃 설정.
	//	클래식은 단순하고 일반적인 레이아웃이며 다양한 디자인에 잘 맞습니다.
	//	로딩 및 팁 위젯을 포함하는 테두리는 하단 또는 상단에 있을 수 있습니다.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash|Layout")
	FNxClassicLayoutSettings Classic;
	
	// 센터 레이아웃 설정.
	//	로딩 위젯은 화면 중앙에 위치하며, 팁 위젯은 하단 또는 상단에 위치할 수 있습니다.
	//	로딩 아이콘이 메인 디자인인 경우 중앙 레이아웃이 좋은 선택입니다.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash|Layout")
	FNxCenterLayoutSettings Center;

	// 레터박스 레이아웃 설정.
	//	레터박스 레이아웃에는 화면 상단과 하단에 두 개의 테두리가 있습니다.
	//	로딩 위젯은 화면 상단에 있고 팁은 화면 하단에 있을 수 있으며, 그 반대도 가능합니다.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash|Layout")
	FNxLetterboxLayoutSettings Letterbox;

	// 사이드바 레이아웃 설정.
	//	사이드바 레이아웃에는 화면 왼쪽이나 오른쪽에 세로 테두리가 있습니다.
	//	사이드바는 팁 위젯의 높이로 인해 스토리텔링, 긴 문단에 적합합니다.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash|Layout")
	FNxSidebarLayoutSettings Sidebar;

	// 듀얼 사이드바 레이아웃 설정
	//	사이드바 레이아웃과 유사하지만 듀얼 사이드바 레이아웃에는 화면 왼쪽과 오른쪽에 두 개의 수직 테두리가 있습니다.
	//	듀얼 사이드바 레이아웃은 팁 위젯의 높이로 인해 스토리텔링, 긴 문단에 적합합니다.
	UPROPERTY(Config, EditAnywhere, Category = "LoadingSplash|Layout")
	FNxDualSidebarLayoutSettings DualSidebar;
};
