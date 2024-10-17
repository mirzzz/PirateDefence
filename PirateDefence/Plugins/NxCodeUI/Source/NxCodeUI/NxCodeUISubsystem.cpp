#include "NxCodeUISubsystem.h"// Fill out your copyright notice in the Description page of Project Settings.
#include "Kismet/GameplayStatics.h"
#include "Slate/SceneViewport.h"

#include "NxGameInstanceBase.h"
#include "NxTableSubsystem.h"

#include "Frame/NxFrameWidget.h"
#include "Table/NxTable_UI.h"
#include "NxCodeUISettings.h"
#include "NxCodeWindowSubsystem.h"
#include "NxCodeWindow.h"
#include "NxCodeWindowsLibrary.h"
#include "NxBaseLog.h"

/* 
 NxCodeUISubsystem 생성시점에 대해서 고민을 많이 했는데,
 자동으로 월드가 변경될때 이벤트를 받아서 처리 할려고 했지만,
 GameViewport가 생성되지 않아서 BeginePlay 함수에서 생성 하도록 수정
*/

/*static*/ UNxCodeUISubsystem* UNxCodeUISubsystem::Get()
{
	return UNxGameInstanceBase::GetGameInstanceSubsystem<UNxCodeUISubsystem>();
}

const FString UNxCodeUISubsystem::sPageName  = TEXT("Page");
const FString UNxCodeUISubsystem::sPopupName = TEXT("Popup");

void UNxCodeUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	NxPrintFunc(LogUI);

	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UNxCodeUISubsystem::Tick), 0.0f);

	if (const UNxCodeUISettings* settings = GetDefault<UNxCodeUISettings>())
	{
		BPPath = settings->BPPath;
		if (nullptr != settings->MessageBoxDlg)
			MessageBoxClassOf = settings->MessageBoxDlg.LoadSynchronous();
		if (nullptr != settings->ProgressIndicatorDlg)
			ProgressIndicatorClassOf = settings->ProgressIndicatorDlg.LoadSynchronous();

		if (nullptr != settings->MessageBoxOneDlg)
			MessageBox2ClassOfs.Add(settings->MessageBoxOneDlg.LoadSynchronous());
		if (nullptr != settings->MessageBoxTwoDlg)
			MessageBox2ClassOfs.Add(settings->MessageBoxTwoDlg.LoadSynchronous());
		if (nullptr != settings->MessageBoxThreeDlg)
			MessageBox2ClassOfs.Add(settings->MessageBoxThreeDlg.LoadSynchronous());

		if (nullptr != settings->NoticesDlg)
			NoticesClassOf = settings->NoticesDlg.LoadSynchronous();
	}

	H_PreLoadMapWithContext = FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::OnPreLoadMap);
	H_PostLoadMapWithWorld  = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMap);

	H_PostWorldCreation = FWorldDelegates::OnPostWorldCreation.AddUObject(this, &ThisClass::OnPostWorldCreation);
	H_PrevWorldInitialization = FWorldDelegates::OnPreWorldInitialization.AddUObject(this, &ThisClass::OnPrevWorldInitialization);
	H_PostWorldInitialization = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInitialization);

	H_SafeFrameChangedEvent = FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &ThisClass::OnSafeFrameChangedEvent);

	FViewport::ViewportResizedEvent.AddUObject(this, &UNxCodeUISubsystem::OnViewportResized);
	  
	//Create(GetWorld());
}

void UNxCodeUISubsystem::Deinitialize()
{
	NxPrintFunc(LogUI);

	FCoreUObjectDelegates::PreLoadMapWithContext.Remove(H_PreLoadMapWithContext);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(H_PostLoadMapWithWorld);

	FWorldDelegates::OnPostWorldCreation.Remove(H_PostWorldCreation);
	FWorldDelegates::OnPreWorldInitialization.Remove(H_PrevWorldInitialization);
	FWorldDelegates::OnPostWorldInitialization.Remove(H_PostWorldInitialization);

	DelegateTick.Unbind();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	Super::Deinitialize();
}

bool UNxCodeUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	NxPrintFunc(LogUI);

	TArray<UClass*> ChildClasses;

	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 다른 곳에 정의된 재정의 구현이 없는 경우에만 인스턴스를 만듭니다. (외부에서 생성시 기본 시스템은 생성하지 않음)
	return ChildClasses.Num() == 0;
}

bool UNxCodeUISubsystem::Create(class UWorld* InWorld)
{
	TArray<TObjectPtr<UNxFrameWidget>> uIFrames;

	//
	if (const UNxCodeUISettings* settings = GetDefault<UNxCodeUISettings>())
	{
		UGameInstance* GameInstance = GetGameInstance();
		UNxCodeWindowSubsystem* NxCodeWindowSubsystem = GameInstance->GetSubsystem<UNxCodeWindowSubsystem>();

		auto SubclassMainFrame = settings->SubclassRootFrame.LoadSynchronous();
		UNxFrameWidget* newUIFrame = CreateWidget<UNxFrameWidget>(InWorld, SubclassMainFrame);
		FVector2D ScreenSize;
		GEngine->GameViewport->GetViewportSize(ScreenSize);
		newUIFrame->Init(ScreenSize);
		newUIFrame->AddToViewport();
		uIFrames.Add(newUIFrame);

		//추가
		for (int i = 1; i < UIFrames.Num(); ++i)
		{
			auto SubclassCommonFrame = settings->SubclassAttachFrame.LoadSynchronous();
			newUIFrame = CreateWidget<UNxFrameWidget>(InWorld, SubclassCommonFrame);
			//UVxMultiWindowsLibrary::AddWidgetToWindow(newUIFrame, MultiWindows[i - 1], 0);
			UNxCodeWindowsLibrary::AddWidgetToWindow(newUIFrame, UIFrames[i]->GetMultiWindow(), 0);
			uIFrames.Add(newUIFrame);
		}
	}

	UIFrames.Empty();
	UIFrames = uIFrames;

	if (UNxTableSubsystem* tableSubsystem = UNxTableSubsystem::Get())
	{
		UDataTable* dataTable = tableSubsystem->GetDataTable(E_DataTable::UI);
		if (dataTable != nullptr)
		{
			WidgetTable.Empty();

			//TArray<FVxTable_UI*> widgetList;
			//dataTable->GetAllRows<FVxTable_UI>(TEXT("GetAllRows"), widgetList);
			TArray<FName> rowNames = dataTable->GetRowNames();			
			for (int i=0; i< rowNames.Num(); ++i )
			{
				FNxTable_UI* widgetInfo = dataTable->FindRow<FNxTable_UI>(rowNames[i], rowNames[i].ToString());
				if(WidgetTable.Contains(widgetInfo->WidgetID) == false )
				{
					WidgetTable.Add(widgetInfo->WidgetID, widgetInfo);
				}
				else
				{
					NxPrintWarning( LogUI, TEXT("UITable에 WidgetID 중복된게 있습니다. %s"), *rowNames[i].ToString() );
				}
			}
		}
	}

	return true;
}

void UNxCodeUISubsystem::RemoveAllFrame()
{
	MapUIWnd.Empty();

	for(int32 i=0; i<UIFrames.Num(); i++)
	{
		if (0 == i)
			UIFrames[i]->RemoveFromParent();
		else
		{
			//UIFrames[i]->RemoveFromParent();
			UNxCodeWindowsLibrary::RemoveWidgetFromWindow(UIFrames[i]);
		}
	}
	UIFrames.Empty();
}

void UNxCodeUISubsystem::Destroy()
{
	RemoveAllFrame();
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	FString Path = MakeFullPath(uiName);

	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI, TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("GetUIFrame fail"));
		return nullptr;
	}

	UClass* Class = LoadClass<UNxActivatableWidgetWindow>(Path);
	if (nullptr == Class)
	{
		NxPrintWarning(LogUI, TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("LoadClass fail"));
		return nullptr;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	return UIFrame->PushWidgetToLayerStack(eLayer, Class);
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(const FString& uiPath, const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	FString Path = MakeFullPath(uiPath, uiName);

	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("GetUIFrame fail"));
		return nullptr;
	}

	UClass* Class = LoadClass<UNxActivatableWidgetWindow>(Path);
	if (nullptr == Class)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("LoadClass fail"));
		return nullptr;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	return UIFrame->PushWidgetToLayerStack(eLayer, Class);
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(TSubclassOf<UNxActivatableWidgetWindow>& uiWidget, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	return Open(uiWidget.Get(), FramePos, eLayer, PreClose);
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(UClass* Class, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	if (nullptr == Class)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() Class is null"));
		return nullptr;
	}

	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *Class->GetName(), TEXT("GetUIFrame fail"));
		return nullptr;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	return UIFrame->PushWidgetToLayerStack(eLayer, Class);
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(E_WidgetID widgetId, bool PreClose /*= false*/)
{
	if (FNxTable_UI* widgetInfo = WidgetTable.FindRef(widgetId))
	{
		const FSoftObjectPath& AssetPath = widgetInfo->WidgetClass.ToSoftObjectPath();
		if (AssetPath.IsValid())
		{
			TSubclassOf<UUserWidget> LoadedSubclass = widgetInfo->WidgetClass.Get();
			if (!LoadedSubclass && UAssetManager::IsInitialized())
			{
				LoadedSubclass = Cast<UClass>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false));
			}

			if (LoadedSubclass)
			{
				if (WidgetInstanceId.Contains(widgetId) == false)
				{
					TObjectPtr<UNxActivatableWidgetWindow> newWidget = Open(LoadedSubclass, (uint8)widgetInfo->AttachFrame, widgetInfo->AttachLayer, PreClose);
					WidgetInstanceId.Add(widgetId, newWidget);
				}
				
				return WidgetInstanceId[widgetId];
			}
			else
			{
				// 계속 노출
				NxMsgArgs(FColor::Red, 0.0f, TEXT("자산 클래스 [%s]을(를) 로드하지 못했습니다."), *widgetInfo->WidgetClass.ToString());
			}
		}
		else
		{
			// Asset 경로 Warning!
		}
	}
	return nullptr;
}

UNxActivatableWidgetWindow* UNxCodeUISubsystem::Open(const FString& tableRowName, bool PreClose /*= false*/)
{
	if (UNxTableSubsystem* tableSubsystem = UNxTableSubsystem::Get())
	{
		if (FNxTable_UI* widgetInfo = tableSubsystem->GetTableRecord<FNxTable_UI>(E_DataTable::UI, tableRowName))
		{
			const FSoftObjectPath& AssetPath = widgetInfo->WidgetClass.ToSoftObjectPath();
			if (AssetPath.IsValid())
			{
				TSubclassOf<UUserWidget> LoadedSubclass = widgetInfo->WidgetClass.Get();
				if (!LoadedSubclass && UAssetManager::IsInitialized())
				{
					LoadedSubclass = Cast<UClass>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false));
				}

				if (LoadedSubclass)
				{
					if (WidgetInstanceName.Contains(tableRowName) == false)
					{
						TObjectPtr<UNxActivatableWidgetWindow> newWidget = Open(LoadedSubclass, (uint8)widgetInfo->AttachFrame, widgetInfo->AttachLayer, PreClose);
						WidgetInstanceName.Add(tableRowName, newWidget);
					}

					return WidgetInstanceName[tableRowName];
				}
				else
				{
					// 계속 노출
					NxMsgArgs(FColor::Red, 0.0f, TEXT("자산 클래스 [%s]을(를) 로드하지 못했습니다."), *widgetInfo->WidgetClass.ToString());
				}
			}
		}
	}
	return nullptr;
}

bool UNxCodeUISubsystem::OpenAsync(const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	FString Path = MakeFullPath(uiName);

	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("GetUIFrame fail"));
		return false;
	}

	UClass* Class = LoadClass<UNxActivatableWidgetWindow>(Path);
	if (nullptr == Class)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("LoadClass fail"));
		return false;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	UIFrame->PushWidgetToLayerStackAsync(eLayer, true, Class);
	return true;
}

bool UNxCodeUISubsystem::OpenAsync(const FString& uiPath, const FString& uiName, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	FString Path = MakeFullPath(uiPath, uiName);

	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("GetUIFrame fail"));
		return false;
	}

	UClass* Class = LoadClass<UNxActivatableWidgetWindow>(Path);
	if (nullptr == Class)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *uiName, TEXT("LoadClass fail"));
		return false;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	UIFrame->PushWidgetToLayerStackAsync(eLayer, true, Class);
	return true;
}

bool UNxCodeUISubsystem::OpenAsync(TSubclassOf<UNxActivatableWidgetWindow>& uiWidget, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() GetUIFrame fail"));
		return false;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	UIFrame->PushWidgetToLayerStackAsync(eLayer, true, uiWidget.Get());
	return true;
}

bool UNxCodeUISubsystem::OpenAsync(UClass* Class, uint8 FramePos, E_StackLayer eLayer, bool PreClose)
{
	UNxFrameWidget* UIFrame = GetUIFrame(FramePos);
	if (nullptr == UIFrame)
	{
		NxPrintWarning(LogUI,  TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *Class->GetName(), TEXT("GetUIFrame fail"));
		return false;
	}

	if (true == PreClose)
		UIFrame->CloseTopWnd(eLayer);

	UIFrame->PushWidgetToLayerStackAsync(eLayer, true, Class);
	return true;
}

bool UNxCodeUISubsystem::OpenAsync(E_WidgetID widgetId, bool bSuspendInputUntilComplete, bool PreClose /*= false*/)
{
	if (FNxTable_UI* widgetInfo = WidgetTable.FindRef(widgetId))
	{
		const FSoftObjectPath& AssetPath = widgetInfo->WidgetClass.ToSoftObjectPath();
		if (AssetPath.IsValid())
		{
			TSubclassOf<UNxActivatableWidgetWindow> LoadedSubclass = widgetInfo->WidgetClass.Get();
			if (!LoadedSubclass && UAssetManager::IsInitialized())
			{
				LoadedSubclass = Cast<UClass>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false));
			}

			if (LoadedSubclass)
			{
				UNxFrameWidget* UIFrame = GetUIFrame((uint8)widgetInfo->AttachFrame);
				if (nullptr == UIFrame)
				{
					NxPrintWarning(LogUI, TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *LoadedSubclass->GetName(), TEXT("GetUIFrame fail"));
					return false;
				}

				if (true == PreClose)
					UIFrame->CloseTopWnd(widgetInfo->AttachLayer);

				//TFunction<void(EAsyncWidgetLayerState, UVxUIBaseWnd*)> StateFunc
				UIFrame->PushWidgetToLayerStackAsync<UNxActivatableWidgetWindow>(widgetInfo->AttachLayer, bSuspendInputUntilComplete, LoadedSubclass.Get(),
					[this, widgetId](E_AsyncWidgetLayerState State, UNxActivatableWidgetWindow* BaseWnd)
					{
						switch (State)
						{
							case E_AsyncWidgetLayerState::AfterPush:
							{
								if (WidgetInstanceId.Contains(widgetId) == false)
								{
									WidgetInstanceId.Add(widgetId, BaseWnd);
								}
								break;
							}
				
							case E_AsyncWidgetLayerState::Canceled:							
							{
								NxMsgArgs(FColor::Yellow, 3.0f, TEXT("자산 클래스 [%s]의 로드가 최소 됨."), *NxEnum_GetString(E_WidgetID, widgetId));
								break;
							}
						}
					}
				);
				return true;
			}
			else
			{
				// 계속 노출
				NxMsgArgs(FColor::Red, 0.0f, TEXT("자산 클래스 [%s]을(를) 로드하지 못했습니다."), *widgetInfo->WidgetClass.ToString());
			}
		}
	}
	return false;
}

bool UNxCodeUISubsystem::OpenAsync(const FString& tableRowName, bool bSuspendInputUntilComplete, bool PreClose /*= false*/)
{
	if (UNxTableSubsystem* tableSubsystem = UNxTableSubsystem::Get())
	{
		if (FNxTable_UI* widgetInfo = tableSubsystem->GetTableRecord<FNxTable_UI>(E_DataTable::UI, tableRowName))
		{
			const FSoftObjectPath& AssetPath = widgetInfo->WidgetClass.ToSoftObjectPath();
			if (AssetPath.IsValid())
			{
				TSubclassOf<UUserWidget> LoadedSubclass = widgetInfo->WidgetClass.Get();
				if (!LoadedSubclass && UAssetManager::IsInitialized())
				{
					LoadedSubclass = Cast<UClass>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false));
				}

				if (LoadedSubclass)
				{
					UNxFrameWidget* UIFrame = GetUIFrame((uint8)widgetInfo->AttachFrame);
					if (nullptr == UIFrame)
					{
						NxPrintWarning(LogUI, TEXT("NxCodeUISubsystem::Open() [%s] - %s"), *LoadedSubclass->GetName(), TEXT("GetUIFrame fail"));
						return false;
					}

					if (true == PreClose)
						UIFrame->CloseTopWnd(widgetInfo->AttachLayer);

					//TFunction<void(EAsyncWidgetLayerState, UVxUIBaseWnd*)> StateFunc
					UIFrame->PushWidgetToLayerStackAsync<UNxActivatableWidgetWindow>(widgetInfo->AttachLayer, bSuspendInputUntilComplete, LoadedSubclass.Get(),
						[this, tableRowName](E_AsyncWidgetLayerState State, UNxActivatableWidgetWindow* BaseWnd)
						{
							switch (State)
							{
								case E_AsyncWidgetLayerState::AfterPush:
								{
									if (WidgetInstanceName.Contains(tableRowName) == false)
									{
										WidgetInstanceName.Add(tableRowName, BaseWnd);
									}
									break;
								}

								case E_AsyncWidgetLayerState::Canceled:
								{
									NxMsgArgs(FColor::Yellow, 3.0f, TEXT("자산 클래스 [%s]의 로드가 최소 됨."), *tableRowName);
									break;
								}
							}
						}
					);
					return true;
				}
				else
				{
					// 계속 노출
					NxMsgArgs(FColor::Red, 0.0f, TEXT("자산 클래스 [%s]을(를) 로드하지 못했습니다."), *widgetInfo->WidgetClass.ToString());
				}
			}
			else
			{
				// Asset 경로 Warning!
			}
		}
	}
	return false;
}

UNxMsgboxDialog* UNxCodeUISubsystem::OpenMessageBox(uint8 FramePos, E_StackLayer eLayer)
{
	UNxActivatableWidgetWindow* UIBaseWnd = Open(MessageBoxClassOf.Get(), FramePos, eLayer);
	return Cast<UNxMsgboxDialog>(UIBaseWnd);
}

UNxMsgboxExtendDialog* UNxCodeUISubsystem::OpenMessageBox(uint8 FramePos, E_StackLayer eLayer, FNxMsgboxButtonInfo& Info)
{
	/*if (false == Info.CheckButtonCount(1))
		return nullptr;*/

	int buttonCount = Info.Buttons.Num() - 1;
	UNxMsgboxExtendDialog* UIMessageBoxWnd2 = Cast<UNxMsgboxExtendDialog>(Open(MessageBox2ClassOfs[buttonCount].Get(), FramePos, eLayer));
	UIMessageBoxWnd2->Setup(Info);
	return UIMessageBoxWnd2;
}

UNxNoticesDialog* UNxCodeUISubsystem::OpenNotices(uint8 FramePos, FText Message, float Duration)
{
	UNxNoticesDialog* UINoticesWnd = Cast<UNxNoticesDialog>(Open(NoticesClassOf.Get(), FramePos, E_StackLayer::Notices));
	UINoticesWnd->Setup(Message, Duration);
	return UINoticesWnd;
}

UNxNoticesDialog* UNxCodeUISubsystem::OpenNotices(uint8 FramePos, FText Message, FOnNoticesDialogCloseDelegate FuncClose)
{
	UNxNoticesDialog* UINoticesWnd = Cast<UNxNoticesDialog>(Open(NoticesClassOf.Get(), FramePos, E_StackLayer::Notices));
	UINoticesWnd->Setup(Message, FuncClose);
	return UINoticesWnd;
}

bool UNxCodeUISubsystem::OpenIndicator(uint8 FramePos)
{
	if (true == IsOpenIndicator(FramePos))
		return false;

	UNxActivatableWidgetWindow* UIBaseWnd = Open(ProgressIndicatorClassOf.Get(), FramePos, E_StackLayer::Indicator);
	if (nullptr == UIBaseWnd)
		return false;

	IndicatorUIWnds.Add(FramePos, Cast<UNxWaitingDialog>(UIBaseWnd));

	return true;
}

void UNxCodeUISubsystem::CloseIndicator(uint8 FramePos)
{
	auto findWnd = IndicatorUIWnds.Find(FramePos);
	if (nullptr != findWnd)
	{
		(*findWnd)->Close();
		IndicatorUIWnds.Remove(FramePos);
	}
}

bool UNxCodeUISubsystem::IsOpenIndicator(uint8 FramePos)
{
	auto findWnd = IndicatorUIWnds.Find(FramePos);
	return nullptr != findWnd ? true : false;
}

void UNxCodeUISubsystem::Close(UNxActivatableWidgetWindow* InWnd)
{
	InWnd->Close();
	//InWnd->RemoveFromParent();
	//InWnd->RemoveFromRoot();
}

void UNxCodeUISubsystem::Close(E_WidgetID widgetId)
{
	if (WidgetInstanceId.Contains(widgetId))
	{
		TObjectPtr<UNxActivatableWidgetWindow> InWnd = WidgetInstanceId.FindRef(widgetId);
		if (InWnd)
		{
			InWnd->Close();
		}
		WidgetInstanceId.Remove(widgetId);
	}
}

void UNxCodeUISubsystem::Close(const FString& tableRowName)
{
	if (WidgetInstanceName.Contains(tableRowName))
	{
		TObjectPtr<UNxActivatableWidgetWindow> InWnd = WidgetInstanceName.FindRef(tableRowName);
		if (InWnd)
		{
			InWnd->Close();
		}
		WidgetInstanceName.Remove(tableRowName);
	}
}

UNxFrameWidget* UNxCodeUISubsystem::AddMultiWindow(class UWorld* InWorld,
	float PosX, float PosY, int32 ResX, int32 ResY, EWindowMode::Type WindowMode)
{
	const UNxCodeUISettings* settings = GetDefault<UNxCodeUISettings>();
	if (nullptr == settings)
		return nullptr;

	auto subclassUI = settings->SubclassAttachFrame.LoadSynchronous();
	UNxFrameWidget* newUIFrame = CreateWidget<UNxFrameWidget>(InWorld, subclassUI);

	UGameInstance* GameInstance = GetGameInstance();
	UNxCodeWindowSubsystem* NxCodeWindowSubsystem = GameInstance->GetSubsystem<UNxCodeWindowSubsystem>();

	UNxCodeWindow* window = NxCodeWindowSubsystem->CreateViewportClient();
	if (nullptr != window)
	{
		float titlebar = 0.f;
		if (EWindowMode::Type::Windowed == WindowMode)
		{
			//titlebar = window->GameViewportClientWindow.Pin()->GetTitleBarSize().Get();
			titlebar = 32;
		}
		window->SetRectWindow(FVector2D(PosX, PosY + titlebar), ResX, ResY, WindowMode);
		window->OnCloseWindow.AddDynamic(this, &UNxCodeUISubsystem::OnCloseWindow);
		//MultiWindows.Add(window);

		UNxCodeWindowsLibrary::AddWidgetToWindow(newUIFrame, window, 0);
	}
	newUIFrame->Init(FVector2D(ResX, ResY), window);

	UIFrames.Add(newUIFrame);

	return newUIFrame;
}

void UNxCodeUISubsystem::RemoveMultiWindow(const UNxCodeWindow* NxCodeWindow)
{
	for (int i = 1; i < UIFrames.Num(); ++i)
	{
		if (UIFrames[i]->GetMultiWindow() == NxCodeWindow)
		{
			UIFrames.RemoveAt(i);
			break;
		}
	}
}

void UNxCodeUISubsystem::OnCloseWindow(const UNxCodeWindow* NxCodeWindow)
{
	RemoveMultiWindow(NxCodeWindow);
}

bool UNxCodeUISubsystem::Tick(float DeltaTime)
{
	DelegateTick.ExecuteIfBound(this, DeltaTime);

	return true;
}

void UNxCodeUISubsystem::OnPreLoadMap(const FWorldContext& InWorldContext, const FString& InMapName)
{
	NxPrintDisplay(LogUI, TEXT("MapName : [%s]"), *InMapName);

	//Create(InWorldContext.World());
}

void UNxCodeUISubsystem::OnPostLoadMap(UWorld* InLoadedWorld)
{
	if (InLoadedWorld)
	{
		NxPrintDisplay(LogUI, TEXT("WorldName : [%s]"), *InLoadedWorld->GetName());

		//Create(InLoadedWorld);
	}
}

void UNxCodeUISubsystem::OnPostWorldCreation(UWorld* World)
{
	// World Partition Action 생성
	NxPrintDisplay(LogUI, TEXT("WorldName : [%s]"), *World->GetName());
}

void UNxCodeUISubsystem::OnPrevWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	NxPrintDisplay(LogUI, TEXT(" [%s]"), *World->GetName());

	//Create(World);
}

void UNxCodeUISubsystem::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	NxPrintDisplay(LogUI, TEXT("NxCodeUISubsystem::OnPostWorldInitialization() [%s]"), *World->GetName());

	//RemoveAllFrame();
	//Create(World);
}

void UNxCodeUISubsystem::OnViewportResized(FViewport* Viewport, uint32 Unused)
{
	FIntPoint Size = Viewport->GetSizeXY();
	NxPrintDisplay(LogUI, TEXT("NxCodeUISubsystem::OnViewportResized() With[%d], Height[%d]"), Size.X, Size.Y);

	//bool find = false;
	//for (int i=1; i<UIFrames.Num(); i++)
	//{
	//	FSceneViewport* sceneViewport = UIFrames[i]->GetMultiWindow()->SceneViewport.Get();
	//	if (sceneViewport == Viewport)
	//	{
	//		UIFrames[i]->ViewportResized(Viewport, Unused);
	//		find = true;
	//		break;
	//	}
	//}

	//if (false == find && UIFrames.Num() > 0)
	//{
	//	UIFrames[0]->ViewportResized(Viewport, Unused);
	//}
}

void UNxCodeUISubsystem::OnSafeFrameChangedEvent()
{
	NxPrintTxt(LogUI, TEXT("NxCodeUISubsystem::OnSafeFrameChangedEvent()"));

//	if (OnSafeFrameChangedEventDelegate.IsBound())
//	{
//		if (GEngine && GEngine->GameViewport)
//		{
//			TWeakPtr<SWindow>		window = GEngine->GameViewport->GetWindow();
//			UGameViewportClient* viewport = GEngine->GameViewport;
//
//			if (viewport)
//			{
//				FIntPoint		 viewSize = viewport->Viewport->GetSizeXY();
//				const FVector2D  viewportCenter = FVector2D(viewSize.X / 2, viewSize.Y / 2);
//
//				OnSafeFrameChangedEventDelegate.Broadcast(viewSize.X, viewSize.Y);
//			}
//		}
//
//		// GameUserSetting 값으로 해상도 변경.
//		//UGameUserSettings* userSettings = GEngine->GetGameUserSettings();
//		//if (ensure(userSettings))
//		//{
//		//	const FIntPoint CurrentRes = userSettings->GetScreenResolution();			
//		//	FSystemResolution::RequestResolutionChange(Width, Height, EWindowMode::Windowed);
//		//}
//	}
}