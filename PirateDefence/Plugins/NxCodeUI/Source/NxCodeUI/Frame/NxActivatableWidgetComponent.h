// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxActivatableWidgetBase.h"
#include "NxActivatableWidgetComponent.generated.h"

class USizeBox;
class UCanvasPanel;
class FVxUI_WndJson;

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

	//~UVxUIBaseActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UVxUIBaseActivatableWidget interface

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

protected:
	//// 이 UI가 활성화되어 있는 동안 사용할 원하는 입력 모드입니다. 
	////	- 예를 들어 키 누름이 여전히 게임/플레이어 컨트롤러에 도달하도록 할것인지 여부.
	//UPROPERTY(EditDefaultsOnly, Category = Input)
	//EVxUI_InputMode InputConfig = EVxUI_InputMode::Default;
};
