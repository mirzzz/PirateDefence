
#include "NxCodeWindowsEngineSubsystem.h"
#include "NxCodeWindowsModule.h"
#include "NxCodeWindow.h"
#include "NxBaseLog.h"


UNxCodeWindowsEngineSubsystem::UNxCodeWindowsEngineSubsystem() : UEngineSubsystem()
{

}

void UNxCodeWindowsEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NxPrintFunc(LogCodeWindows);	
}

void UNxCodeWindowsEngineSubsystem::Deinitialize()
{
	Super::Deinitialize();
	NxPrintFunc(LogCodeWindows);
}

void UNxCodeWindowsEngineSubsystem::RedrawViewports(bool bShouldPresent)
{
	NxPrintFunc(LogCodeWindows);
}