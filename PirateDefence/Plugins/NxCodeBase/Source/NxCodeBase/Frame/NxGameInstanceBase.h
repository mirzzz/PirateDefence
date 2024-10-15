// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UObject/NoExportTypes.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "NxGameInstanceBase.generated.h"


// 해상도 변경 이벤트.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSafeFrameChangedEvent, int32, width, int32, height);

/**
 * UNxGameInstanceBase
 */
UCLASS(Config = Game)
class NXCODEBASE_API UNxGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UNxGameInstanceBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	// Editor상 Play/Simulate 게임 초기화 호출됨.
	virtual FGameInstancePIEResult InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params) override;

	// Editor상 Play/Simulate 게임 시작 호출됨.
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif

protected:

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void ReturnToMainMenu() override;

	virtual void InitPrintDefineLog();


public:

	// Convenience Static Function		
	//------------------------------------------------------------------	
	
	UFUNCTION(BlueprintCallable, Category = Frame)
	static class UNxTableSubsystem* TableSubsystem();
		
	UFUNCTION(BlueprintCallable, Category = Frame)
	static class UNxWorldSubsystemBase* WorldSubsystem();

	UFUNCTION(BlueprintCallable, Category = Frame)
	static class UNxWorldPartitionSubsystemBase* WorldPartitionSubsystem();


	// Convenience Event Function
	//------------------------------------------------------------------	
	bool bRegistEvent = true;

	virtual void RegistEventdetection();
	virtual void UnregistEventdetection();

	bool OnEvent_Ticker(float deltaTime);

	void OnEvent_PreLoadMap(const FWorldContext& InWorldContext, const FString& InMapName);
	void OnEvent_PostLoadMap(class UWorld* InLoadedWorld);

	void OnEvent_AppHasReactivated();
	void OnEvent_AppWillDeactivate();
	void OnEvent_AppHasEnteredForeground();
	void OnEvent_AppWillEnterBackground();
	void OnEvent_AppLicenseChange();

	void OnEvent_SafeFrameChangedEvent();

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = Frame)
	FOnSafeFrameChangedEvent OnSafeFrameChangedEventDelegate;

	void OnEvent_ControllerConnectionChange(EInputDeviceConnectionState InConnectionState, FPlatformUserId InPlatformUserId, FInputDeviceId InInputDeviceId);

	// Editor - Actor Notify
#if WITH_EDITOR
	void OnEvent_ActorSpawned(AActor* Actor);
	void OnEvent_ActorDeleted(AActor* Actor);
	void OnEvent_LevelActorListChanged();
#endif 

protected:

	FTSTicker::FDelegateHandle	H_Ticker;

	FDelegateHandle		H_PreLoadMapWithContext;
	FDelegateHandle		H_PostLoadMapWithWorld;

	FDelegateHandle		H_AppHasReactivated;
	FDelegateHandle		H_AppWillDeactivate;
	FDelegateHandle		H_AppHasEnteredForeground;			// App Resume 
	FDelegateHandle		H_AppWillEnterBackground;			// App Suspend
	FDelegateHandle		H_AppLicenseChange;

	FDelegateHandle		H_SafeFrameChangedEvent;			// 해상도 변경
	FDelegateHandle		H_ControllerConnectionChange;		// 게임패드 변경

	// Editor - Actor Notify
#if WITH_EDITOR
	FDelegateHandle		H_ActorSpawned;			
	FDelegateHandle		H_ActorDeleted;			
	FDelegateHandle		H_LevelActorListChanged;	
#endif 

protected:
	
	static UNxGameInstanceBase* BaseInstance;

public:
	template<typename T>
	static T* GetGameInstanceSubsystem()
	{
		return nullptr != BaseInstance ? BaseInstance->GetSubsystem<T>() : nullptr;
	}

	// UWorldSubsystem() 반환
	template<typename T>
	static T* GetWorldSubSystem()
	{
		if (nullptr == BaseInstance)
			return nullptr;

		UWorld* World = BaseInstance->GetWorld();
		if (nullptr == World)
			return nullptr;

		return World->GetSubsystem<T>();
	}

	//  ULocalPlayerSubsystem() 반환
	template<typename T>
	static T* GetLocalPlayerSubSystem()
	{
		if (nullptr == BaseInstance)
			return nullptr;

		ULocalPlayer* localPlayer = BaseInstance->GetFirstGamePlayer();
		if (nullptr == localPlayer)
			return nullptr;

		return localPlayer->GetSubsystem<T>();
	}
};
