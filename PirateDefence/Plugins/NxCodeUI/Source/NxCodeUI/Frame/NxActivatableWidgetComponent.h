// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxActivatableWidgetBase.h"
#include "NxActivatableWidgetComponent.generated.h"

class USizeBox;
class UCanvasPanel;

/**
 * 
 */
UCLASS(Abstract)
class NXCODEUI_API UNxActivatableWidgetComponent : public UNxActivatableWidgetBase
{
	GENERATED_BODY()
	
public:
	UNxActivatableWidgetComponent(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool Initialize() override;

	//only at runtime
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;
	virtual void NativeOnActivated() override;
	virtual	void NativeOnDeactivated() override;

	virtual bool ProcessInputKey(FInputKeyEventArgs& EventArgs);
	UFUNCTION()
	virtual FEventReply ProcessMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()
	virtual FEventReply ProcessMouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	//~UNxActivatableWidgetBase interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UNxActivatableWidgetBase interface

	virtual bool Open(bool bHorizontal) override;
	virtual void Close() override;
	virtual void UpdateControl() override;

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif

private:
	virtual void InitCollectionChild() override;		// Anim, Widget 정보 수집
	virtual void InitLocalization()  override;
	virtual void UpdateStyle();
};
