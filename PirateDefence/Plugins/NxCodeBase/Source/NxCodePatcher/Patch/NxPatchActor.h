// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NxPatchActor.generated.h"

class UVxPatchSubsystem;

// Patcher ���� ����
//	- ��Ī�� �Ϸ�Ǹ�, Start ����.
UCLASS()
class NXCODEPATCHER_API ANxPatchActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ANxPatchActor();

protected:
	
	virtual void BeginPlay() override;
		
public:	
	
	// ��ó �ý���
	class UNxPatchSubsystem* GetPatchSubsystem();

	// ��ġ�� ����Ǹ� ȣ���.
	UFUNCTION()
	void PatchComplete(bool Succeeded);

	virtual void Tick(float DeltaTime) override;
};
