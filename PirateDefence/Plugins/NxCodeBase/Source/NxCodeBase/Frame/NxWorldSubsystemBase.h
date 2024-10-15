// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NxWorldSubsystemBase.generated.h"

/**
 * UNxWorldSubsystemBase
 */
UCLASS()
class NXCODEBASE_API UNxWorldSubsystemBase : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	static UNxWorldSubsystemBase* Get();

	// 하위에 어떤 시스템을 생성할지 제어가 필요시 재정의, 서버에서만 필요한 시스템 생성등이 필요한 경우.
	// 인스턴스가 생성되기 전에 CDO에서 호출 된다.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 시스템 인스턴스 초기화를 위해 구현
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 시스템 인스턴스의 초기화 해제를 위해 구현
	virtual void Deinitialize() override;

	// 모든 UWorldSubsystems이 초기화되면 호출
	virtual void PostInitialize() override;

	// 게임 모드가 올바른 상태로 전환 되고
	// - 모든 액터에서 BeginPlay를 호출하기 전에 World가 게임플레이를 시작할 준비가 되면 호출됩니다.
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// 월드 구성 요소가 업데이트된 후 호출됩니다 
	virtual void OnWorldComponentsUpdated(UWorld& World) override;

	// 스트리밍 레벨 업데이트 (World의 UpdateStreamingState 함수에 의해 호출됨) - Tick
	virtual void UpdateStreamingState() override;

	// Tick - UTickableWorldSubsystem 사용안하고, TimeEvent 사용해서 처리함.
	//virtual void Tick(float DeltaTime) override;

	// World Delegate Function
	virtual void OnPrevWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
	virtual void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);

	virtual void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	virtual void OnPostWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	virtual void OnPostWorldCreation(UWorld* World);	
	virtual void OnPrevWorldFinishDestroy(UWorld* World);

	virtual void OnLevelAddedToWorld(ULevel* Level, UWorld* World);
	virtual void OnLevelRemovedFromWorld(ULevel* Level, UWorld* World);

protected:

	// 지형 Proxy 갯수 확인.
	TArray<TWeakObjectPtr<class ALandscapeStreamingProxy>> LandscapeStreamingProxys;

	// World Delegate
	FDelegateHandle		HW_PrevWorldInitialization;			// World Init Prev
	FDelegateHandle		HW_PostWorldInitialization;			// World Init Post

	FDelegateHandle		HW_WorldCleanup;					// World CleanUp Start		
	FDelegateHandle		HW_PostWorldCreation;				// World Create
	
	FDelegateHandle		HW_PostWorldCleanup;				// World CleanUp End
	FDelegateHandle		HW_PrevWorldFinishDestroy;			// World Destruction

	FDelegateHandle		HW_LevelAddedToWorld;				// World Level Add
	FDelegateHandle		HW_LevelRemovedFromWorld;			// World Level Remove	

};
