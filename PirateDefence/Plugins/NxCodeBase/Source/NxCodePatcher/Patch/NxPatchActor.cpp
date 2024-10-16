// Fill out your copyright notice in the Description page of Project Settings.


#include "NxPatchActor.h"
#include "Engine/GameInstance.h"
#include "NxPatchSubsystem.h"
#include "NxBaseLog.h"

ANxPatchActor::ANxPatchActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

UNxPatchSubsystem* ANxPatchActor::GetPatchSubsystem()
{
	if (UGameInstance* gameInstance = GetGameInstance())
	{
		UNxPatchSubsystem* patchSubsystem = gameInstance->GetSubsystem<UNxPatchSubsystem>();
		return patchSubsystem;
	}
	return nullptr;
}

void ANxPatchActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetPatchSubsystem())
	{
		GetPatchSubsystem()->OnAutoPatchComplete.AddDynamic(this, &ThisClass::PatchComplete);
		GetPatchSubsystem()->AutoPatchBegin();
	}
}

void ANxPatchActor::PatchComplete(bool Succeeded)
{
	if (Succeeded == false)
	{
		// 패치가 실패해도 StartMap으로 우선 넘겨준다.
		NxPrintWarning(LogBase, TEXT("Patching Error"));
	}

	// 패치가 완료되면 StartMap 시작.
	if (GetPatchSubsystem())
	{		
		GetPatchSubsystem()->OnAutoPatchComplete.RemoveDynamic(this, &ThisClass::PatchComplete);
		GetPatchSubsystem()->GotoNextLevel();
	}
}

void ANxPatchActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetPatchSubsystem())
	{
		GetPatchSubsystem()->AutoPatchUpdate(DeltaTime);
	}

}

