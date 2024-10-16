// Fill out your copyright notice in the Description page of Project Settings.


#include "NxMountPakDLC.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANxMountPakDLC::ANxMountPakDLC()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultPath = TEXT("../../../PirateDefence/Content/Paks/");
}
 
// Called when the game starts or when spawned
void ANxMountPakDLC::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
void ANxMountPakDLC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ANxMountPakDLC::OpenDLC(FString PluginName, FName MapName)
{
	// .pak 파일 검사 없이 에디터 내부에서 Map 열기 활성
	if (bEnableInEditor && GetWorld()->WorldType == EWorldType::PIE)
	{
		bCanOpenMap = true;
	}
	else
	{
		// DLC Pak 파일 검사
		FString fullpath = FPaths::ConvertRelativePathToFull(DefaultPath + PluginName + ".pak");

		bCanOpenMap = FPaths::FileExists(fullpath);
	}

	if (bCanOpenMap)
	{
		UGameplayStatics::OpenLevel(GetWorld(), MapName);
	}
}

