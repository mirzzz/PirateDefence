// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeWindowSubsystem.h"
#include "NxCodeWindowsModule.h"
#include "GameFramework/HUD.h"
#include "NxCodeWindowsManager.h"
#include "NxCodeWindowsLibrary.h"
#include "NxBaseLog.h"


void UNxCodeWindowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	NxPrintFunc(LogCodeWindows);

	Super::Initialize(Collection);

	// MultiView Manager 접근
	UNxCodeWindowsLibrary::GetMultiWindowsManager(MultiManager);
	if (MultiManager != nullptr)
	{
		// 보조 윈도우 변경 감지 콜백 등록.
		MultiManager->OnSubWindowsArrayChanged.AddDynamic(this, &UNxCodeWindowSubsystem::OnAncillaryWindowsArrayChanged);
	}

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UNxCodeWindowSubsystem::Tick), 0.0f);
}

void UNxCodeWindowSubsystem::Deinitialize()
{
	NxPrintFunc(LogCodeWindows);

	Super::Deinitialize();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool UNxCodeWindowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	NxPrintFunc(LogCodeWindows);

	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 다른 곳에 정의된 재정의 구현이 없는 경우에만 인스턴스를 만듭니다. (외부에서 생성시 기본 시스템은 생성하지 않음)
	return ChildClasses.Num() == 0;
}

// 보조 윈도위 배열이 변경되면 호출됨.
void UNxCodeWindowSubsystem::OnAncillaryWindowsArrayChanged(const UNxCodeWindowsManager* MultiWindowsManager)
{
	for (auto& windowitem : MultiWindowsManager->SubWindows)
	{
		UNxCodeWindow* window = windowitem;
		
		NxPrintLog(LogCodeWindows, TEXT("Window Name : %s"), *window->GetWindowTitle().ToString());
	}
}

UNxCodeWindow* UNxCodeWindowSubsystem::CreateViewportClient()
{
	if (nullptr == MultiManager)
		return nullptr;

	UNxCodeWindow* newWindow = MultiManager->CreateGameViewportClientWindow();
	if (nullptr == newWindow)
		return nullptr;

	newWindow->ClearViews();

	// Window Title 설정.
	bool	windowIsValid = false;
	int32	windowIndex = 0;
	newWindow->GetWindowIndex(windowIndex, windowIsValid);

	FString newWindowTitle = FString::Printf( TEXT("WindowIndex : %d"), windowIndex);
	newWindow->SetWindowTitle(FText::FromString(newWindowTitle));

	Windows.Add( newWindow );

	return newWindow;
}

bool UNxCodeWindowSubsystem::Tick(float DeltaTime)
{

	return true;
}
