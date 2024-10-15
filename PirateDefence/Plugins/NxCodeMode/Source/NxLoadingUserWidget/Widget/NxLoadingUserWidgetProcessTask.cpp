// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxLoadingUserWidgetProcessTask.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "NxLoadingUserWidgetSubsystem.h"

/*static*/ UNxLoadingUserWidgetProcessTask* UNxLoadingUserWidgetProcessTask::CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason)
{
	UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;

	UNxLoadingUserWidgetSubsystem* widgetLoadingSubsystem = gameInstance ? gameInstance->GetSubsystem<UNxLoadingUserWidgetSubsystem>() : nullptr;

	if (widgetLoadingSubsystem)
	{
		UNxLoadingUserWidgetProcessTask* newLoadingTask = NewObject<UNxLoadingUserWidgetProcessTask>(widgetLoadingSubsystem);

		newLoadingTask->SetShowLoadingScreenReason(ShowLoadingScreenReason);

		widgetLoadingSubsystem->RegisterLoadingProcessor(newLoadingTask);
		
		return newLoadingTask;
	}

	return nullptr;
}

void UNxLoadingUserWidgetProcessTask::Unregister()
{
	UNxLoadingUserWidgetSubsystem* loadingScreenSys = Cast<UNxLoadingUserWidgetSubsystem>(GetOuter());
	loadingScreenSys->UnregisterLoadingProcessor(this);
}

void UNxLoadingUserWidgetProcessTask::SetShowLoadingScreenReason(const FString& InReason)
{
	Reason = InReason;
}

bool UNxLoadingUserWidgetProcessTask::ShouldShowLoadingScreen(FString& OutReason) const
{
	OutReason = Reason;
	return true;
}