// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/OnlineReplStructs.h"
#include "NxStateUnit.h"
#include "NxStateMachineUnit.h"
#include "NxStateMachineComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AActor;

DECLARE_DELEGATE(FOnStateMachineFinishDelegate)

/**
 * FSM 시스템 
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class NXCODEBASE_API UNxStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNxStateMachineComponent(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	virtual void BeginPlay() override;

	/** UActorComponent */
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */


	UFUNCTION(BlueprintCallable, Category = Golf)
	void SwitchStateByKey(FString StateKey);

	UFUNCTION(BlueprintCallable, Category = Golf)
	void SwitchState(UNxStateUnit* NewState);
	
	UFUNCTION(BlueprintCallable, Category = Golf)
	void PlayStateMachine();

	UFUNCTION(BlueprintCallable, Category = Golf)
	void PauseStateMachine(bool stop);

	// Next State가 null일 경우 호출되는 이벤트 - FSM 종료됨
	void SetStateMachineFinishSignalIfNextIsNull(FOnStateMachineFinishDelegate callback);
	FOnStateMachineFinishDelegate	OnMachineFisinshIfNextisNull;

	// Before State가 null일 경우 호출되는 이벤트 - FSM 앞으로 이동 불가
	void SetStateMachineFinishSignalIfBeforeIsNull(FOnStateMachineFinishDelegate callback);
	FOnStateMachineFinishDelegate	OnMachineFisinshIfBeforeisNull;

	UNxStateUnit* GetStartState();
	UNxStateUnit* GetNextState();
	UNxStateUnit* GetBeforeState();

	UNxStateUnit* GetState(FString stateName);
	UNxStateUnit* GetState(UClass* stateClass);


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	FString MachineName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	FString InitialState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	TMap<FString, TSubclassOf<UNxStateUnit>> AvailableStates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	bool bDebug = false;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, Category = State, meta = (ClampMin="0", ClampMax = "10", UIMin="0", UIMax="10") )
	int32 StateHistoryLength = 0;

	// 디테일 창에 노출
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = State)
	TArray<UNxStateUnit*> StateHistory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, AdvancedDisplay, Category = State)
	UNxStateUnit* CurrentState = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, AdvancedDisplay, Category = State)
	TMap<FString, UNxStateUnit*> StateMap;

protected:
	bool bCanTickState = false;

	// 등록된 상태 생성.
	void CreateStates();
};
