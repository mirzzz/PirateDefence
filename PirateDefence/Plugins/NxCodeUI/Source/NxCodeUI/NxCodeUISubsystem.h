// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
	      
#include "NxCodeUIUtility.h"
#include "NxFrameWidget.h"
#include "NxActivatableWidgetWindow.h"
#include "NxPageWindow.h"
#include "NxPopupWindow.h"
#include "NxMsgboxDialog.h"
#include "NxMsgboxExtendDialog.h"
#include "NxNoticesDialog.h"
#include "NxWaitingDialog.h"
#include "NxUIEnum.h"

#include "NxCodeUISubsystem.generated.h"

// 멀티뷰 Window Support Class
class UNxCodeWindow;

DECLARE_DELEGATE_TwoParams(FOnCodeUISystemTickDelegate, UNxCodeUISubsystem* uiSystem, float deltaTime);

/**
 *
 */
UCLASS()
class NXCODEUI_API UNxCodeUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UNxCodeUISubsystem* Get();

	static const FString sPageName;
	static const FString sPopupName;

	static FString MakeFullPath_Static(const FString& InPath, const FString& InUIame) {
		return InPath / InUIame + TEXT(".") + InUIame + "_C";
	}

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

public:
	bool Create(class UWorld* InWorld);
	void Destroy();

	// Multiview
	void RemoveAllFrame();

	//Open
	UNxActivatableWidgetWindow* Open(const FString& uiName, uint8 FramePos, E_StackLayer eLayer,  bool PreClose = false);
	UNxActivatableWidgetWindow* Open(const FString& uiPath, const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	UNxActivatableWidgetWindow* Open(TSubclassOf<UNxActivatableWidgetWindow>& uiWidget, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	UNxActivatableWidgetWindow* Open(UClass* Class, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	//Open-UseTable
	UNxActivatableWidgetWindow* Open(E_WidgetID widgetId,  bool PreClose = false);
	UNxActivatableWidgetWindow* Open(const FString& tableRowName, bool PreClose = false);

	//OpenAsync
	bool OpenAsync(const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	bool OpenAsync(const FString& uiPath, const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	bool OpenAsync(TSubclassOf<UNxActivatableWidgetWindow>& uiWidget, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	bool OpenAsync(UClass* Class, uint8 FramePos, E_StackLayer eLayer, bool PreClose = false);
	//OpenAysnc-UseTable
	bool OpenAsync(E_WidgetID widgetId, bool bSuspendInputUntilComplete = true, bool PreClose = false);
	bool OpenAsync(const FString& tableRowName, bool bSuspendInputUntilComplete = true, bool PreClose = false);

	UNxMsgboxDialog*		OpenMessageBox(uint8 FramePos, E_StackLayer eLayer);
	UNxMsgboxExtendDialog*	OpenMessageBox(uint8 FramePos, E_StackLayer eLayer, FNxMsgboxButtonInfo& Info);
	UNxNoticesDialog*		OpenNotices(uint8 FramePos, FText Message, float Duration);
	UNxNoticesDialog*		OpenNotices(uint8 FramePos, FText Message, FOnNoticesDialogCloseDelegate FuncClose);

	bool OpenIndicator(uint8 FramePos);
	void CloseIndicator(uint8 FramePos);
	bool IsOpenIndicator(uint8 FramePos);

	//Close
	void Close(UNxActivatableWidgetWindow* InWnd);
	//Close-UseTable
	UNxActivatableWidgetWindow* GetWidgetUI(E_WidgetID widgetId)			{ return WidgetInstanceId.FindRef(widgetId); }
	UNxActivatableWidgetWindow* GetWidgetUI(const FString& tableRowName)	{ return WidgetInstanceName.FindRef(tableRowName); }
	void Close(E_WidgetID widgetId);
	void Close(const FString& tableRowName);

	//
	TArray<TObjectPtr<UNxFrameWidget>>& GetUIFrames() {
		return UIFrames;
	}
	UNxFrameWidget* GetUIFrame(int32 Pos) {
		if (UIFrames.Num() <= Pos)
			return nullptr;
		return UIFrames[Pos].Get();
	};

	int32 GetCountFrame() {
		return UIFrames.Num();
	}

	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	ActivatableWidgetT* GetUIWnd(int32 FramePos, E_StackLayer EnumLayer) {
		if (UIFrames.Num() <= FramePos)
			return nullptr;
		return UIFrames[FramePos]->GetWidgetToLayerStack<ActivatableWidgetT>(EnumLayer);
	};

	template <typename ActivatableWidgetT = UNxActivatableWidgetWindow>
	ActivatableWidgetT* GetUIWnd() {

		for (int32 frameNo = 0; frameNo < UIFrames.Num(); ++frameNo)
		{
			for (int32 layerNo = 0; layerNo < (int32)E_StackLayer::Max; ++layerNo)
			{
				ActivatableWidgetT* findWidget = GetUIWnd<ActivatableWidgetT>(frameNo, (E_StackLayer)layerNo);
				if (nullptr != findWidget)
					return findWidget;
			}
		}
		return nullptr;
	};

	UNxFrameWidget* AddMultiWindow(class UWorld* InWorld, float PosX, float PosY, int32 ResX, int32 ResY, EWindowMode::Type WindowMode);
	void RemoveMultiWindow(const UNxCodeWindow* NxCodeWindow);

	UFUNCTION()
	void OnCloseWindow(const UNxCodeWindow* NxCodeWindow);

	//Tick
	FOnCodeUISystemTickDelegate DelegateTick;

private:
	bool Tick(float DeltaTime);

	FTSTicker::FDelegateHandle TickHandle;

protected:
	template <typename T>
	UClass* LoadClass(const FString& Path);

	template <typename T>
	T* LoadWndUI(const FString& Name, bool Dialog = true);

	FORCEINLINE FString MakeFullPath(const FString& InUIame) {
		return BPPath / InUIame + TEXT(".") + InUIame + "_C";
	}

	FORCEINLINE FString MakeFullPath(const FString& InPath, const FString& InUIame) {
		return BPPath / InPath / InUIame + TEXT(".") + InUIame + "_C";
	}

protected:
	FDelegateHandle		H_PreLoadMapWithContext;
	FDelegateHandle		H_PostLoadMapWithWorld;

	void OnPreLoadMap(const FWorldContext& InWorldContext, const FString& InMapName);
	void OnPostLoadMap(class UWorld* InLoadedWorld);

	FDelegateHandle		H_PostWorldCreation;
	FDelegateHandle		H_PrevWorldInitialization;			// World Init Prev
	FDelegateHandle		H_PostWorldInitialization;			// World Init Post

	void OnPostWorldCreation(UWorld* World);
	void OnPrevWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
	void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);

	FDelegateHandle		H_SafeFrameChangedEvent;			// 해상도 변경
	void OnSafeFrameChangedEvent();
	void OnViewportResized(FViewport* Viewport, uint32 Unused);

protected:
	UPROPERTY() TArray<TObjectPtr<UNxFrameWidget>> UIFrames;
	UPROPERTY() TMap<FString, UNxActivatableWidgetWindow*> MapUIWnd;

	//Popup
	UPROPERTY() FString BPPath;
	UPROPERTY() TSubclassOf<UNxMsgboxDialog> MessageBoxClassOf;
	UPROPERTY() TSubclassOf<UNxWaitingDialog> ProgressIndicatorClassOf;
	UPROPERTY() TArray<TSubclassOf<UNxMsgboxExtendDialog>> MessageBox2ClassOfs;
	UPROPERTY() TSubclassOf<UNxNoticesDialog> NoticesClassOf;

	// 
	UPROPERTY() TObjectPtr<UNxWaitingDialog> IndicatorUIWnd;
	UPROPERTY()	TMap<uint8, TObjectPtr<UNxWaitingDialog>> IndicatorUIWnds;

	// Table ListUp
	TMap<E_WidgetID, struct FNxTable_UI*>		WidgetTable;
	UPROPERTY() TMap<E_WidgetID, TObjectPtr<UNxActivatableWidgetWindow>>	WidgetInstanceId;
	UPROPERTY() TMap<FString,	 TObjectPtr<UNxActivatableWidgetWindow>>	WidgetInstanceName;

	////Multiview
	//UPROPERTY(Transient)
	//TArray<UNxCodeWindow*>  MultiWindows;
};

template <typename T>
UClass* UNxCodeUISubsystem::LoadClass(const FString& Path)
{
	FSoftClassPath ClassRef(Path);
	UClass* UClassPtr = ClassRef.TryLoadClass<T>();
	return UClassPtr;
}

template <typename T>
T* UNxCodeUISubsystem::LoadWndUI(const FString& InName, bool Dialog)
{
	UNxActivatableWidgetWindow** FindUIWnd = MapUIWnd.Find(InName);
	if (nullptr != *FindUIWnd)
		return Cast<T>(*FindUIWnd);

	FString Path = MakeFullPath(InName);
	//FString Path(BPPath);
	//Path += TEXT("/");
	//Path += Dialog ? sDialogName : sPopupName;
	//Path += TEXT("/");
	//Path += Name;

	T* UIWnd = Nx_UtilityUI::LoadWidget<T>(GetOuter(), GetWorld(), Path);
	if (nullptr != UIWnd)
		MapUIWnd.Add(InName, UIWnd);
	return UIWnd;
}