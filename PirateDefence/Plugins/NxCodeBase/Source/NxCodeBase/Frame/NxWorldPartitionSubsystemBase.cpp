// Fill out your copyright notice in the Description page of Project Settings.


#include "NxWorldPartitionSubsystemBase.h"
#include "NxGameInstanceBase.h"
#include "GameFramework/WorldSettings.h"
#include "EngineUtils.h"

#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "NxBaseLog.h"


/*static*/ UNxWorldPartitionSubsystemBase* UNxWorldPartitionSubsystemBase::Get()
{
	return UNxGameInstanceBase::GetWorldSubSystem<UNxWorldPartitionSubsystemBase>();
}

bool UNxWorldPartitionSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
	NxPrintFunc(LogBase);

	bool success = Super::ShouldCreateSubsystem(Outer);
	if (success == false)
		return false;

	if (UWorld* WorldOuter = Cast<UWorld>(Outer))
	{
		// 월드 세팅에서 활성화 여부를 선택할수도 있음.
		if (AWorldSettings* worldSettings = Cast<AWorldSettings>(WorldOuter->GetWorldSettings()))
		{
			
		}
	}

	return true;
}

// 월드가 새로 로딩 될떄 - 초기화 진행
void UNxWorldPartitionSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//FString worldname = GetWorld()->GetName();
	//NxPrintTet(LogBase, TEXT("[------------------------------------------------------ NxWorldPartitionSubsystemBase Initialize]"));	
	//NxPrintLog(LogBase, TEXT("[ Start World : %s"), *worldname);

	HW_PostWorldCreation		= FWorldDelegates::OnPostWorldCreation.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnPostWorldCreation);
	HW_PrevWorldInitialization	= FWorldDelegates::OnPreWorldInitialization.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnPrevWorldInitialization);
	HW_PostWorldInitialization	= FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnPostWorldInitialization);
	 
	HW_WorldCleanup				= FWorldDelegates::OnWorldCleanup.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnWorldCleanup);
	HW_PostWorldCleanup			= FWorldDelegates::OnPostWorldCleanup.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnPostWorldCleanup);
	HW_PrevWorldFinishDestroy	= FWorldDelegates::OnPreWorldFinishDestroy.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnPrevWorldFinishDestroy);

	HW_LevelAddedToWorld		= FWorldDelegates::LevelAddedToWorld.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnLevelAddedToWorld);
	HW_LevelRemovedFromWorld	= FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UNxWorldPartitionSubsystemBase::OnLevelRemovedFromWorld);

}

// 월드가 새로 로딩 될떄 - 초기화 해제
void UNxWorldPartitionSubsystemBase::Deinitialize()
{
	// World Delegate 해제
	FWorldDelegates::OnPostWorldCreation.Remove(HW_PostWorldCreation);
	FWorldDelegates::OnPreWorldInitialization.Remove(HW_PrevWorldInitialization);
	FWorldDelegates::OnPostWorldInitialization.Remove(HW_PostWorldInitialization);

	FWorldDelegates::OnWorldCleanup.Remove(HW_WorldCleanup);
	FWorldDelegates::OnPostWorldCleanup.Remove(HW_PostWorldCleanup);
	FWorldDelegates::OnPreWorldFinishDestroy.Remove(HW_PrevWorldFinishDestroy);

	FWorldDelegates::LevelAddedToWorld.Remove(HW_LevelAddedToWorld);
	FWorldDelegates::LevelRemovedFromWorld.Remove(HW_LevelRemovedFromWorld);

	//FString worldname = GetWorld()->GetName();
	//NxPrintLog(LogBase, TEXT("[ Destroy World : %s"), *worldname);
	//NxPrintTxt(LogBase, TEXT("[---------------------------------------------------- NxWorldPartitionSubsystemBase Deinitialize]"));	

	Super::Deinitialize();
}

// 월드 시스템 객체 생성 
void UNxWorldPartitionSubsystemBase::PostInitialize()
{
	NxPrintFunc(LogBase);
	Super::PostInitialize();
}

// 월드 객체 모두 배치된 후 호출됨
void UNxWorldPartitionSubsystemBase::OnWorldBeginPlay(UWorld& InWorld)
{
	NxPrintFunc(LogBase);
	Super::OnWorldBeginPlay(InWorld);

	for (TActorIterator<ALandscape> iter(GetWorld()); iter; ++iter)
	{
		ALandscape* land = *iter;
	}
}

// Component Update
void UNxWorldPartitionSubsystemBase::OnWorldComponentsUpdated(UWorld& World)
{
	Super::OnWorldComponentsUpdated(World);
}

// Steaming Update
void UNxWorldPartitionSubsystemBase::UpdateStreamingState()
{
	Super::UpdateStreamingState();
}

void UNxWorldPartitionSubsystemBase::OnPrevWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)	{/*NxPrintFunc(LogBase);*/}
void UNxWorldPartitionSubsystemBase::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)	{/*NxPrintFunc(LogBase);*/}
void UNxWorldPartitionSubsystemBase::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)			{/*NxPrintFunc(LogBase);*/}
void UNxWorldPartitionSubsystemBase::OnPostWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)		{/*NxPrintFunc(LogBase);*/}
void UNxWorldPartitionSubsystemBase::OnPrevWorldFinishDestroy(UWorld* World)											{/*NxPrintFunc(LogBase);*/}
void UNxWorldPartitionSubsystemBase::OnPostWorldCreation(UWorld* World)													{/*NxPrintFunc(LogBase);*/}

void UNxWorldPartitionSubsystemBase::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{	
}

void UNxWorldPartitionSubsystemBase::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
}

