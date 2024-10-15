// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxStateMachineComponent.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"

#include "NxBaseLog.h"

UNxStateMachineComponent::UNxStateMachineComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);

	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNxStateMachineComponent::OnRegister()
{
	Super::OnRegister();

	// Editor 디테일 창에 노출지원
	if (AActor* owner = Cast<AActor>(GetOwner()))
	{
		owner->AddInstanceComponent(this);
	}
}

void UNxStateMachineComponent::OnUnregister()
{
	if (AActor* owner = Cast<AActor>(GetOwner()))
	{
		owner->RemoveInstanceComponent(this);
	}

	Super::OnUnregister();
}

void UNxStateMachineComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UNxStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// State 생성 및 초기화
	CreateStates();
}

void UNxStateMachineComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCanTickState)
	{
		if(CurrentState)
			CurrentState->TickState(DeltaTime);
	}

	if (bDebug)
	{
		// History Output.
		if (StateHistory.Num() > 0)
		{
			for (int i =0; i< StateHistory.Num(); ++i)
			{
				NxMsgArgs(FColor::Orange, 0.0f, TEXT("[%2d]  %s:%s"), i, *MachineName, *StateHistory[i]->GetName());
			}
		}
	}
}

void UNxStateMachineComponent::SwitchStateByKey(FString StateKey)
{
	if(StateKey.IsEmpty())
		return;

	UNxStateUnit* NewState = StateMap.FindRef( StateKey );

	if (NewState && NewState->IsValidLowLevel())
	{
		if (CurrentState == nullptr)
		{
			CurrentState = NewState;
		}
		else
		{
			// 현재 State와 같은 State 일 경우
			if (CurrentState->GetClass() == NewState->GetClass() && CurrentState->bCanRepeat == false)
			{
				if (bDebug)
				{
					NxMsgArgs(FColor::Red, 3.0f, TEXT("[%s] State switch failed, by not repeatable."), *CurrentState->GetStateName());
				}
			}
			else
			{
				bCanTickState = false;

				CurrentState->ExitState();

				if (StateHistory.Num() < StateHistoryLength)
				{
					StateHistory.Push( CurrentState );
				}
				else
				{
					if( StateHistory.Num() )
						StateHistory.RemoveAt(0);

					StateHistory.Push(CurrentState);
				}

				CurrentState = NewState;
			}
		}

		if (CurrentState)
		{
			CurrentState->EnterState(GetOwner());
			bCanTickState = true;
		}
	}
	else
	{
		NxMsgArgs(FColor::Red, 3.0f, TEXT("[%s] State switch Failed, by Invalid State."), *MachineName);
	}
}

void UNxStateMachineComponent::SwitchState(UNxStateUnit* NewState)
{
	if(NewState == nullptr)
		return;

	FString state = NewState->GetStateName();
	SwitchStateByKey(state);
}

void UNxStateMachineComponent::PlayStateMachine()
{
	SwitchStateByKey(InitialState);
}

void UNxStateMachineComponent::PauseStateMachine(bool stop)
{
	bCanTickState = !stop;
}

// FSM 종료시 이벤트 발생
void UNxStateMachineComponent::SetStateMachineFinishSignalIfNextIsNull(FOnStateMachineFinishDelegate callback)
{
	OnMachineFisinshIfNextisNull = callback;
}

void UNxStateMachineComponent::SetStateMachineFinishSignalIfBeforeIsNull(FOnStateMachineFinishDelegate callback)
{
	OnMachineFisinshIfBeforeisNull = callback;
}

UNxStateUnit* UNxStateMachineComponent::GetStartState()
{
	return StateMap.FindRef(InitialState);
}

UNxStateUnit* UNxStateMachineComponent::GetNextState()
{
	if (CurrentState)
	{
		UNxStateUnit* nextState = StateMap.FindRef(CurrentState->GetNextStateName());

		// 다음 State가 없을때, FSM Finish Delegate 있으면 호출.
		if(nextState == nullptr && OnMachineFisinshIfNextisNull.IsBound() == true)
		{
			OnMachineFisinshIfNextisNull.Execute();
			OnMachineFisinshIfNextisNull.Unbind();
		}

		return nextState;
	}
	return nullptr;
}

UNxStateUnit* UNxStateMachineComponent::GetBeforeState()
{
	if (CurrentState)
	{
		UNxStateUnit* beforeState = StateMap.FindRef(CurrentState->GetBeforeStateName());

		// 다음 State가 없을때, FSM Finish Delegate 있으면 호출.
		if (beforeState == nullptr && OnMachineFisinshIfBeforeisNull.IsBound() == true)
		{
			OnMachineFisinshIfBeforeisNull.Execute();
			OnMachineFisinshIfBeforeisNull.Unbind();
		}

		return beforeState;
	}
	return nullptr;
}

UNxStateUnit* UNxStateMachineComponent::GetState(FString stateName)
{
	return StateMap.FindRef(stateName);
}

UNxStateUnit* UNxStateMachineComponent::GetState(UClass* stateClass)
{
	for (auto& state : StateMap)
	{
		if (state.Value->StaticClass() == stateClass)
			return state.Value;
	}
	return nullptr;
}

// FSM State 생성.
void UNxStateMachineComponent::CreateStates()
{
	NxPrintLog(LogState, TEXT("LogicState Create, %s"), *GetOwner()->GetName());

	if (StateMap.Num())
	{
		NxPrintWarning(LogState, TEXT("The CrateState() is called Again, Calling Empty() and then ReCreating it. %s"), *GetOwner()->GetName());
		StateMap.Empty();
	}

	UNxStateUnit* beforeState = nullptr;

	for (auto& state : AvailableStates)
	{
		UClass* typeClass = state.Value;
		if(typeClass)
		{
			if (UNxStateUnit* newState = NewObject<UNxStateUnit>(GetOuter(), typeClass))
			{
				newState->SetStateName(state.Key);

				if (beforeState)
				{
					newState->SetBeforeStateName(beforeState->GetStateName());	// 전 State 이름 설정.
					beforeState->SetNextStateName(state.Key);					// 후 State 이름 설정.	
				}

				StateMap.Add(state.Key, newState);

				beforeState = newState;
			}
		}
		else
		{
			NxPrintWarning(LogState, TEXT("LogicState Is Null, %s:%s"), *GetOwner()->GetName(), *state.Key);
		}
	}
}

