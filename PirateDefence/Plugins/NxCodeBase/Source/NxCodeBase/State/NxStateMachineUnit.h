// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "NxStateUnit.h"
#include "NxStateMachineUnit.generated.h"

/** 
 * State Unit Machine 기본 클래스	
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxStateMachineUnit : public UNxStateUnit
{
	GENERATED_BODY()

public:

	friend class UNxStateMachineComponent;
	
	UNxStateMachineUnit();

	void Start();
	void Tick(float DeltaTime);
	void End();

	UFUNCTION(BlueprintCallable, Category = State)
	void SwitchStateByKey(const FString& StateKey );

	UFUNCTION(BlueprintCallable, Category = State)
	void SwitchState(UNxStateUnit* NewState);

	UFUNCTION(BlueprintCallable, Category = State)
	void PlayStateMachine();
	
	virtual FString GetStateName() override;

	UNxStateUnit* GetStartState();
	UNxStateUnit* GetNextState();

	UNxStateUnit* GetState(FString stateName);
	UNxStateUnit* GetState(UClass* stateClass);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	FString InitialState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	TMap<FString, TSubclassOf<UNxStateUnit>> AvailableStates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State)
	bool bDebug = false;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = State, meta = (ClampMin="0", ClampMax = "10", UIMin="0", UIMax="10") )
	int32 StateHistoryLength = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State)
	TArray<UNxStateUnit*> StateHistory;

	UPROPERTY(VisibleAnywhere, Transient, Category = State)
	UNxStateUnit* CurrentState = nullptr;

	UPROPERTY(VisibleAnywhere, Transient, Category = State)
	TMap<FString, UNxStateUnit*> StateMap;

protected:
	bool bCanTickState = false;

	void InitializeStates();
};
