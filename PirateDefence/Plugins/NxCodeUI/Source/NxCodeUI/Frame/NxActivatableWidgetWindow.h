// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxActivatableWidgetBase.h"
#include "NxActivatableWidgetWindow.generated.h"

class USizeBox;
class UCanvasPanel;
class UNxActivatableWidgetComponent;

/**
 * 
 */
UCLASS(Abstract)
class NXCODEUI_API UNxActivatableWidgetWindow : public UNxActivatableWidgetBase
{
	GENERATED_BODY()
	
public:
	UNxActivatableWidgetWindow(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool Initialize() override;

	//only at runtime
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;
	virtual void NativeOnActivated() override;
	virtual	void NativeOnDeactivated() override;

	virtual bool ProcessInputKey(FInputKeyEventArgs& EventArgs) override;

	UFUNCTION()
	virtual FEventReply ProcessMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	//~UVxUIBaseActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UVxUIBaseActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

	virtual bool Open(bool bHorizontal) override;
	virtual void Close() override;
	virtual void UpdateControl() override;
	//virtual void CloseWnd();

public:
	template<typename T>
	FORCEINLINE_DEBUGGABLE T* GetComponent(const TCHAR* InComponentName)
	{
		return Cast<T>(GetWidget(InComponentName));
	}

	TArray<UNxActivatableWidgetComponent*> GetComponents();

private:
	virtual void InitCollectionChild();		// Anim, Widget 정보 수집
	virtual void InitLocalization();
	virtual void UpdateStyle();

protected:
	FOverrideInputKeyHandler BeforeInputKeyHandler;

	//Option
	uint8 bRootBorderClickClose : 1;

	//Widget
public:
	void	Load(FVxUI_WndJson& InJson);
	UPanelWidget* GetSlot(const FString& InName);

	void	SetPosition(FVector2D InPos);
	void	SetSize(FVector2D InSize);

	//protected:
	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> RootCanvasPanel;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidgetOptional))
	TObjectPtr<class UCommonBorder> RootBorder;
};
