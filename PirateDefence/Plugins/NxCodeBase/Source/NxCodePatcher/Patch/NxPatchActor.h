// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NxPatchActor.generated.h"

class UVxPatchSubsystem;

// Patcher 실행 액터
//	- 패칭이 완료되면, Start 진행.
UCLASS()
class NXCODEPATCHER_API ANxPatchActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ANxPatchActor();

protected:
	
	virtual void BeginPlay() override;
		
public:	
	
	// 패처 시스템
	class UNxPatchSubsystem* GetPatchSubsystem();

	// 패치가 종료되면 호출됨.
	UFUNCTION()
	void PatchComplete(bool Succeeded);

	virtual void Tick(float DeltaTime) override;
};
