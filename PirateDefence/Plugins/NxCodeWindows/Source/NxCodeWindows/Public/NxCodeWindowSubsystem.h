// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NxCodeWindowSubsystem.generated.h"

class UUserWidget;
class UNxCodeWindow;
class UNxCodeWindowsManager;

/* 
* 보조 화면 관리자. (Multi Window)
*  - 코드 구현 필요.
*/
UCLASS()
class NXCODEWINDOWS_API UNxCodeWindowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UNxCodeWindow* CreateViewportClient();

	UFUNCTION()
	void OnAncillaryWindowsArrayChanged(const UNxCodeWindowsManager* MultiWindowsManager);

	UNxCodeWindowsManager* GetMultiManager() { return MultiManager; }

private:
	bool Tick(float DeltaTime);
	
	FTSTicker::FDelegateHandle TickHandle;

protected:
	
	UPROPERTY(Transient)
	UNxCodeWindowsManager*		MultiManager = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UNxCodeWindow>> Windows;
};