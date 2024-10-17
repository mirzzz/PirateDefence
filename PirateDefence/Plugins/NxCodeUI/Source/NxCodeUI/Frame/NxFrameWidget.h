// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"
#include "NxCodeUILibrary.h"
#include "NxActivatableWidgetWindow.h"
#include "NxActivatableWidgetStack.h"
#include "NxFrameWidget.generated.h"

class UNxCodeWindow;


UCLASS(BlueprintType, Blueprintable, meta = (DisableNativeTick))
class NXCODEUI_API UNxFrameWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UNxFrameWidget(const FObjectInitializer& ObjectInitializer);

	void NativeOnInitialized() override;
	void NativeConstruct() override;

	void Init(FVector2D InScreenSize, UNxCodeWindow* InVxWindow = nullptr);
		
	// PushWidget
	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(
		E_StackLayer Layer,
		bool bSuspendInputUntilComplete,
		TSoftClassPtr<UNxActivatableWidgetWindow> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(Layer, bSuspendInputUntilComplete, ActivatableWidgetClass, [](E_AsyncWidgetLayerState, ActivatableWidgetT*) {});
	}

	// PushWidget 함수포인터지원
	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(
		E_StackLayer Layer,
		bool bSuspendInputUntilComplete,
		TSoftClassPtr<UNxActivatableWidgetWindow> ActivatableWidgetClass,
		TFunction<void(E_AsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UNxActivatableWidgetWindow>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UNxCodeUILibrary::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();

		// Widget를 비동기로 로드하
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(
			ActivatableWidgetClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(this, [this, Layer, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
				{
					// Stream Delegate 설정
					UNxCodeUILibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

					ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(
						Layer,
						ActivatableWidgetClass.Get(),
						[StateFunc](ActivatableWidgetT& WidgetToInit)
						{
							StateFunc(E_AsyncWidgetLayerState::Initialize, &WidgetToInit);
						}
					);

					StateFunc(E_AsyncWidgetLayerState::AfterPush, Widget);
				})
		);

		// 이 핸들러가 취소되면 입력을 재개할 수 있도록 취소 대리자를 설정합니다.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
			[this, StateFunc, SuspendInputToken]()
			{
				UNxCodeUILibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
				StateFunc(E_AsyncWidgetLayerState::Canceled, nullptr);
			})
		);

		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	ActivatableWidgetT* PushWidgetToLayerStack(
		E_StackLayer EnumLayer,
		UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(EnumLayer, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	ActivatableWidgetT* PushWidgetToLayerStack(
		E_StackLayer EnumLayer,
		UClass* ActivatableWidgetClass,
		TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UNxActivatableWidgetWindow>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UNxActivatableWidgetStack* Layer = GetLayer(EnumLayer))
		{
			ActivatableWidgetT* widget = Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
			widget->Open(IsWindowHorizontal());
			return widget;
		}

		return nullptr;
	}

	//
	void RemoveWidgetFromLayer(class UNxActivatableWidgetWindow* UIWnd);

	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	ActivatableWidgetT* GetWidgetToLayerStack(E_StackLayer EnumLayer)
	{
		if (UNxActivatableWidgetStack* Layer = GetLayer(EnumLayer))
		{
			for (auto& wnd : Layer->GetWnds())
			{
				ActivatableWidgetT* castWnd = Cast<ActivatableWidgetT>(wnd);
				if (nullptr != castWnd)
					return castWnd;
			}
		}

		return nullptr;
	}

	//
	bool IsOpenUI(const FString& InUIName);
	UNxActivatableWidgetWindow* GetTopUI(E_StackLayer Layer);

	UNxActivatableWidgetWindow* GetModal() { return ModalUIWnd; }
	void SetModal(UNxActivatableWidgetWindow* UIWnd) { ModalUIWnd = UIWnd; }
	bool IsModal() { return ModalUIWnd ? true : false; }

	void CloseTopWnd(E_StackLayer Layer);

	//Layer
	void InitLayer();
	class UNxActivatableWidgetStack* GetLayer(E_StackLayer Layer);
	void ClearLayer(E_StackLayer Layer);
	UFUNCTION(BlueprintCallable, Category = "VxUI")
	void RegisterLayer(UNxActivatableWidgetStack* LayerWidget);

	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);
	void ViewportResized(FViewport* Viewport, uint32 Unused);
	void ChangeScreenAspectRatio(bool Horizontal);

	FVector2D	GetWindowSize();
	bool		IsWindowHorizontal();

	//MultiWindow
	UNxCodeWindow* GetMultiWindow();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> Canvas_Root;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Game;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_GameMenu;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Menu;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Dialog;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Notices;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Indicator;

	UPROPERTY(BlueprintReadOnly, Category = "VxUI", meta = (BindWidget))
	TObjectPtr<class UNxActivatableWidgetStack> Layer_Dev;

	TArray<FName> SuspendInputTokens;

	UPROPERTY()
	TArray<TObjectPtr<class UNxActivatableWidgetStack>> Layers;

	UPROPERTY()
	TObjectPtr<UNxActivatableWidgetWindow> ModalUIWnd;

	UPROPERTY()
	TObjectPtr<UNxCodeWindow> MultiWindow;	//null is main

	FVector2D ScreenSize;
};
