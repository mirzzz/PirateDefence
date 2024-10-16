// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NxMountPakDLC.generated.h"
 
/*
* Pak 이 있는지 확인하고, 있을경우 Mount 및 Open 
*	- Project Launcher 로 개별 Package 된 Pak 에서 맵로딩.
*/
UCLASS() 
class NXCODEPATCHER_API ANxMountPakDLC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANxMountPakDLC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "DLC")
	FString DefaultPath;
		
	UPROPERTY(EditAnywhere, Category = "DLC")
	bool bEnableInEditor = true;

	UFUNCTION(BlueprintCallable, Category ="DLC")
	void OpenDLC(FString PluginName, FName MapName);

private:
	bool bCanOpenMap = false;

};
