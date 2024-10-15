// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxStateUnit.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateEnterDelegate, UNxStateUnit*, state);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateExitDelegate, UNxStateUnit*, state);

/** 
 * State Flow 기본 클래스	
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxStateUnit : public UObject
{
	GENERATED_BODY()

public:

	friend class UNxStateMachineComponent;
	friend class UNxStateMachineUnit;
	
	UNxStateUnit();
	
	virtual void  EnterState(class AActor* owner);	
	virtual void  TickState(float deltaTime) {}
	virtual void  ExitState();
		
	virtual bool  NextState();

	// 현재 상태를 문자열로 출력 
	virtual FString GetStateName();
	void SetStateName(FString stateName);

	// Before/Next State 이름
	const FString&	GetNextStateName()		{ return NextStateName; }
	const FString&  GetBeforeStateName()	{ return BeforeStateName; }

	void			SetBeforeStateName(FString stateName) { BeforeStateName = stateName; }
	void			SetNextStateName(FString stateName )  { NextStateName = stateName; }

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = State)
	FOnStateEnterDelegate OnStateEnter;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = State)
	FOnStateExitDelegate  OnStateExit;

protected:

	// 업데이트 Tick 여부
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = State)
	bool	bCanTickState = false;

	// 반복 여부
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = State)
	bool	bCanRepeat = false;

	// State 이름
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = State)
	FString	 StateName;

	// 전 State 이름
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = State)
	FString  BeforeStateName;

	// 후 State 이름
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = State)
	FString  NextStateName;

	UPROPERTY()
	TObjectPtr<class AActor>	Owner;

};
