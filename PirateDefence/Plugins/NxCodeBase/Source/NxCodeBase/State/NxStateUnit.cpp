// Fill out your copyright notice in the Description page of Project Settings.

#include "State/NxStateUnit.h"
#include "State/NxStateMachineComponent.h"
#include "GameFramework/Actor.h"
#include "NxBaseLog.h"


UNxStateUnit::UNxStateUnit()
{
	bCanTickState = false;
}

FString UNxStateUnit::GetStateName()
{
	return StateName;
}

void UNxStateUnit::SetStateName(FString stateName)
{
	StateName = stateName;
}

void UNxStateUnit::EnterState(class AActor* owner)
{
	Owner = owner;
	
	if (Owner)
	{
		FString stateMsg = FString::Printf(TEXT("EnterState %s > [%s:%s]  "), *GetStateName(), *Owner->GetName(), *GetName());
		NxMsg(FColor::Green, 5.0f, stateMsg);
	}
}

void UNxStateUnit::ExitState()
{
	bCanTickState = false;

	if (Owner)
	{
		FString stateMsg = FString::Printf(TEXT("ExitState %s > [%s:%s]  "), *GetStateName(), *Owner->GetName(), *GetName());
		NxMsg(FColor::Green, 5.0f, stateMsg);
	}
}

bool UNxStateUnit::NextState()
{
	if(Owner == nullptr)
		return false;

	if (UNxStateMachineComponent* machine = Owner->FindComponentByClass<UNxStateMachineComponent>())
	{
		// 다음 State가 있을 경우 넘김.
		if (machine->GetNextState())
		{
			machine->SwitchState(machine->GetNextState());
			return true;
		}
	}
	return false;
}
