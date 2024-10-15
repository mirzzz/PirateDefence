// Fill out your copyright notice in the Description page of Project Settings.

#include "NxStateMachineUnit.h"
#include "NxStateUnit.h"
#include "NxBaseLog.h"

UNxStateMachineUnit::UNxStateMachineUnit()
{
	
}

void UNxStateMachineUnit::Start()
{
	InitializeStates();
}

void UNxStateMachineUnit::Tick(float DeltaTime)
{
	if (bCanTickState)
	{
		if (CurrentState)
			CurrentState->TickState(DeltaTime);
	}
}

void UNxStateMachineUnit::End()
{
	StateMap.Empty();
	StateHistory.Empty();
}

// FSM State 생성.
void UNxStateMachineUnit::InitializeStates()
{
	UNxStateUnit* beforeState = nullptr;

	for (auto& state : AvailableStates)
	{
		UClass* typeClass = state.Value;

		if (UNxStateUnit* newState = NewObject<UNxStateUnit>(GetOuter(), typeClass))
		{
			newState->SetStateName(state.Key);
			if (beforeState)
			{
				newState->SetNextStateName(beforeState->GetStateName());
			}
			StateMap.Add(state.Key, newState);

			beforeState = newState;
		}
	}
}

// First State 시작.
void UNxStateMachineUnit::PlayStateMachine()
{
	SwitchStateByKey(InitialState);
}

FString UNxStateMachineUnit::GetStateName()
{
	if (CurrentState)
	{
		return Super::GetStateName() + " > " + CurrentState->GetStateName();
	}

	return Super::GetStateName();
}

void UNxStateMachineUnit::SwitchStateByKey(const FString& StateKey)
{
	UNxStateUnit* NewState = StateMap.FindRef(StateKey);

	if (NewState && NewState->IsValidLowLevel())
	{
		if (CurrentState == nullptr)
		{
			CurrentState = NewState;
		}
		else
		{
			// 현재 State 와 같은 State 일 경우
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
					StateHistory.Push(CurrentState);
				}
				else
				{
					StateHistory.RemoveAt(0);
					StateHistory.Push(CurrentState);
				}

				CurrentState = NewState;
			}
		}

		if (CurrentState)
		{
			CurrentState->EnterState(Owner);
			bCanTickState = true;
		}
	}
	else
	{
		NxMsgArgs(FColor::Red, 3.0f, TEXT("[%s] State switch Failed, by Invalid State."), *GetStateName());
	}
}

void UNxStateMachineUnit::SwitchState(UNxStateUnit* NewState)
{
	if (NewState == nullptr)
		return;

	FString state = NewState->GetStateName();
	SwitchStateByKey(state);
}

UNxStateUnit* UNxStateMachineUnit::GetStartState()
{
	return StateMap.FindRef(InitialState);
}

UNxStateUnit* UNxStateMachineUnit::GetNextState()
{
	if (CurrentState)
	{
		return StateMap.FindRef(CurrentState->GetNextStateName());
	}
	return nullptr;
}

UNxStateUnit* UNxStateMachineUnit::GetState(FString stateName)
{
	return StateMap.FindRef(stateName);
}

UNxStateUnit* UNxStateMachineUnit::GetState(UClass* stateClass)
{
	for (auto& state : StateMap)
	{
		if (state.Value->StaticClass() == stateClass)
			return state.Value;
	}
	return nullptr;
}
 