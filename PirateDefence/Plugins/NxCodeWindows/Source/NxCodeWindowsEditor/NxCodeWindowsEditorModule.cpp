
#include "NxCodeWindowsEditorModule.h"
#include "NxBaseLog.h"

DEFINE_LOG_CATEGORY(LogCodeWindowEditor);

IMPLEMENT_MODULE(FNxCodeWindowsEditorModule, NxCodeWindowsEditor)

void FNxCodeWindowsEditorModule::StartupModule()
{
	NxPrintFunc(LogCodeWindowEditor);
}

void FNxCodeWindowsEditorModule::ShutdownModule()
{
	NxPrintFunc(LogCodeWindowEditor);
}
