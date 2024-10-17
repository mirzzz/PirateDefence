
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxCodeView.h"
#include "NxCodeViewManager.generated.h"


/**
 * 뷰 관리자.
 */
USTRUCT(BlueprintType, Blueprintable, meta = (ShortTooltip = ""))
struct NXCODEWINDOWS_API FNxCodeViewManager
{
public:
	GENERATED_USTRUCT_BODY()
	
	FNxCodeViewManager()
	{

	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows|MultiViews|ViewManager")
	TArray<FNxCodeView> Views;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NxCodeWindows|MultiViews|ViewManager")
	bool EnableMultiViews = false;
};
