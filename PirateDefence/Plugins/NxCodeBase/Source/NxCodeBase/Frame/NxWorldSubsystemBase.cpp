// Fill out your copyright notice in the Description page of Project Settings.


#include "NxWorldSubsystemBase.h"
#include "NxGameInstanceBase.h"
#include "GameFramework/WorldSettings.h"
#include "EngineUtils.h"

#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "NxBaseLog.h" 


/*static*/ UNxWorldSubsystemBase* UNxWorldSubsystemBase::Get()
{
	return UNxGameInstanceBase::GetWorldSubSystem<UNxWorldSubsystemBase>();
}

bool UNxWorldSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
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
void UNxWorldSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString worldname = GetWorld()->GetName();
	NxPrintTxt(LogBase, TEXT("[--------------------------------------------------------------- NxWorldSubsystemBase Initialize]"));
	NxPrintLog(LogBase, TEXT("[ Start World : %s"), *worldname);
	
	HW_PostWorldCreation		= FWorldDelegates::OnPostWorldCreation.AddUObject(this, &UNxWorldSubsystemBase::OnPostWorldCreation);
	HW_PrevWorldInitialization	= FWorldDelegates::OnPreWorldInitialization.AddUObject(this, &UNxWorldSubsystemBase::OnPrevWorldInitialization);
	HW_PostWorldInitialization	= FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UNxWorldSubsystemBase::OnPostWorldInitialization);
	 
	HW_WorldCleanup				= FWorldDelegates::OnWorldCleanup.AddUObject(this, &UNxWorldSubsystemBase::OnWorldCleanup);
	HW_PostWorldCleanup			= FWorldDelegates::OnPostWorldCleanup.AddUObject(this, &UNxWorldSubsystemBase::OnPostWorldCleanup);
	HW_PrevWorldFinishDestroy	= FWorldDelegates::OnPreWorldFinishDestroy.AddUObject(this, &UNxWorldSubsystemBase::OnPrevWorldFinishDestroy);

	HW_LevelAddedToWorld		= FWorldDelegates::LevelAddedToWorld.AddUObject(this, &UNxWorldSubsystemBase::OnLevelAddedToWorld);
	HW_LevelRemovedFromWorld	= FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UNxWorldSubsystemBase::OnLevelRemovedFromWorld);

}

// 월드가 새로 로딩 될떄 - 초기화 해제
void UNxWorldSubsystemBase::Deinitialize()
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

	FString worldname = GetWorld()->GetName();
	NxPrintLog(LogBase, TEXT("[ Destroy World : %s"), *worldname);
	NxPrintTxt(LogBase, TEXT("[---------------------------------------------------- NxWorldSubsystemBase Deinitialize]"));
	Super::Deinitialize();
}

// 월드 시스템 객체 생성 
void UNxWorldSubsystemBase::PostInitialize()
{
	Super::PostInitialize();
}

// 월드 객체 모두 배치된 후 호출됨
void UNxWorldSubsystemBase::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

// Component Update
void UNxWorldSubsystemBase::OnWorldComponentsUpdated(UWorld& World)
{
	Super::OnWorldComponentsUpdated(World);
}

// Steaming Update
void UNxWorldSubsystemBase::UpdateStreamingState()
{
	Super::UpdateStreamingState();
}

void UNxWorldSubsystemBase::OnPrevWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)	{ /*NxPrintFunc(LogBase);*/ }
void UNxWorldSubsystemBase::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)	{ /*NxPrintFunc(LogBase);*/ }
void UNxWorldSubsystemBase::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)			{ /*NxPrintFunc(LogBase);*/ }
void UNxWorldSubsystemBase::OnPostWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)		{ /*NxPrintFunc(LogBase);*/ }
void UNxWorldSubsystemBase::OnPrevWorldFinishDestroy(UWorld* World)												{ /*NxPrintFunc(LogBase);*/ }
void UNxWorldSubsystemBase::OnPostWorldCreation(UWorld* World)													{ /*NxPrintFunc(LogBase);*/ }

void UNxWorldSubsystemBase::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{
	// World Partition Actor 추가
	if (Level && Level->IsValidLowLevel())
	{
		for(auto& actor : Level->Actors)
		{
			if (actor->IsA(ALandscapeStreamingProxy::StaticClass()))
			{
				ALandscapeStreamingProxy* LSP = Cast<ALandscapeStreamingProxy>(actor);
				if( LandscapeStreamingProxys.Find(LSP) == INDEX_NONE)
				{
					LandscapeStreamingProxys.Add(LSP);
					NxPrintLog(LogBase, TEXT("OnLevelAddedToWorld() - %s [%d]"), *LSP->GetActorLabel(), LandscapeStreamingProxys.Num());
				}
			}
		}
	}
}

void UNxWorldSubsystemBase::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
	// World Partition Actor 제거
	if (Level && Level->IsValidLowLevel())
	{
		for (auto& actor : Level->Actors)
		{
			if (actor->IsA(ALandscapeStreamingProxy::StaticClass()))
			{
				ALandscapeStreamingProxy* LSP = Cast<ALandscapeStreamingProxy>(actor);
				if (LandscapeStreamingProxys.Find(LSP) != INDEX_NONE)
				{
					LandscapeStreamingProxys.Remove(LSP);
					NxPrintLog(LogBase, TEXT("OnLevelRemovedFromWorld() - %s [%d]"), *LSP->GetActorLabel(), LandscapeStreamingProxys.Num());
				}
			}
		}
	}
}

