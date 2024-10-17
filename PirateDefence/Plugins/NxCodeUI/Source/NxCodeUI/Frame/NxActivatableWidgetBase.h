// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "NxUIEnum.h"
#include "NxActivatableWidgetBase.generated.h"

class USizeBox;
class UCanvasPanel;
class FVxUI_WndJson;
class UVxUIBaseComponent;

/**
 * 
 */
UCLASS(Abstract)
class NXCODEUI_API UNxActivatableWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	static const FName gAniName_Open;
	static const FName gAniName_Close;
	static const FName gAniName_Horizontal;
	static const FName gAniName_Vertical;

	UNxActivatableWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool Initialize() override;

	//only at runtime
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;
	virtual void NativeOnActivated() override;
	virtual	void NativeOnDeactivated() override;


	virtual bool Open(bool bHorizontal);
	virtual void Close();
	virtual void UpdateControl();

	virtual bool ProcessInputKey(FInputKeyEventArgs& EventArgs);

	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

	// Animation 처리
	bool  IsExistAnimation(const FName& animationName);
	bool  IsPlayingAnimation(const FName& animationName);

	UWidgetAnimation* PlayAnimation(const FName& animationName, float startTime = 0.0f, int32 loopCount = 1, EUMGSequencePlayMode::Type playType = EUMGSequencePlayMode::Forward);
	UWidgetAnimation* StopAnimation(const FName& animationName);
	UWidgetAnimation* PauseAnimation(const FName& animationName);
	float GetAnimation_CurrentTime(const FName& animationName);
	float GetAnimation_EndTime(const FName& animationName);

	void  GetAnimationList(OUT TArray<FName>& animationList);

	FWidgetAnimationDynamicEvent OnStartEvent;
	FWidgetAnimationDynamicEvent OnEndEvent;

	UFUNCTION()
	virtual void WidgetAnimationStarted();
	UFUNCTION()
	virtual void WidgetAnimationFinished();

	UWidgetAnimation* CurrentWidgetAnimation;

public:
	template<typename T>
	FORCEINLINE_DEBUGGABLE T* GetWidget(FString& InWidgetName)
	{
		T* find = Cast<T>(GetWidgetFromName(FName(*InWidgetName)));
		if (!find)
		{
			//UE_LOG(VxUserWidgetLog, Warning, TEXT("Cannot find %s."), InWidgetName);
			return nullptr;
		}
		return find;
	}

protected:
	virtual void InitCollectionChild();		// Anim, Widget 정보 수집
	virtual void InitLocalization();

	virtual void RefreshSizeMargin();

protected:
	//
	UPROPERTY(EditAnywhere, Category = Widget)
	E_ScreenOrientation ScreenOrientation;

	//Size 
	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Size : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Size"))
	FVector2D Widget_Size;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Padding : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Padding"))
	FMargin Widget_Padding;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	FOverrideInputKeyHandler BeforeInputKeyHandler;

	UWidgetBlueprintGeneratedClass* BPClass;				// Blueprint Class
	TMap<FName, UWidget*>			IncludeWidgets;			// 포함된 Widget 수집
	TMap<FName, UWidgetAnimation*>	IncludeAnimations;		// 포함된 Anim 수집

	// 이 UI가 활성화되어 있는 동안 사용할 원하는 입력 모드입니다. 
	//	- 예를 들어 키 누름이 여전히 게임/플레이어 컨트롤러에 도달하도록 할것인지 여부.
	UPROPERTY(EditDefaultsOnly, Category = Input)
	E_CommonInputMode InputMode = E_CommonInputMode::Menu;

	// 게임이 입력될 때 원하는 마우스 동작. 
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
