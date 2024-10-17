// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "NxUserWidget.generated.h"

class USizeBox;
class UPanelWidget;
class UNxActivatableWidgetWindow;

USTRUCT()
struct NXCODEUI_API FNxTexturePos// : public UObject
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture> Texture;

	UPROPERTY(EditAnywhere)
	FVector2D Size = FVector2D::ZeroVector;
};

/**
 * 
  */
UCLASS(Abstract)
class NXCODEUI_API UNxUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UNxUserWidget();

	// UUserWidget~
	virtual bool Initialize() override;				// CDO
	virtual void RemoveFromParent() override;		// CDO

	// AddToViewport() 함수가 호출될 때 UI위젯을 생성/소멸 하기 위해 호출됨.
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void SetVisible(bool bVisible);

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

	FVector2D  GetMousePosition();
	FVector2D  GetWidgetCenterPosition(UWidget* parentWidget, UWidget* widget);		// Absolute Position (절대 위치)

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

protected:
	virtual void CollectionChild();
	virtual void RefreshSizeMargin();

protected:
	UWidgetBlueprintGeneratedClass* BPClass;				// Blueprint Class
	TMap<FName, UWidget*>			IncludeWidgets;			// 포함된 Widget 수집
	TMap<FName, UWidgetAnimation*>	IncludeAnimations;		// 포함된 Anim 수집

public:
	void	Load(UNxActivatableWidgetWindow* InParentWnd);
	UPanelWidget* GetSlot(const FString& InName);
	void	CloseParentLayer();

protected:
	TWeakObjectPtr<UNxActivatableWidgetWindow>	ParentWnd;

	//TextureInfo Ex
	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Images : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Images"))
	TArray<FNxTexturePos> Widget_TextureInfos;

	//Text Ex
	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Texts : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Texts"))
	TArray<FText> Widget_Texts;

	//Size 
	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Size : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Size"))
	FVector2D Widget_Size;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_Padding : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (editcondition = "bOverride_Padding"))
	FMargin Widget_Padding;

	//
	UPROPERTY(BlueprintReadOnly, Category = Widget, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(BlueprintReadOnly, Category = Widget, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_Root;
};