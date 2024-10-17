
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

NXCODEWINDOWSEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogCodeWindowEditor, All, All);

// Log On:1/Off:0 적용
#define Show_LogCodeWindowEditor 1

/**
 * The public interface to this module
 */
class INxCodeWindowsEditorModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline INxCodeWindowsEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked< INxCodeWindowsEditorModule >("NxCodeWindowsEditor");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "NxCodeWindowsEditor" );
	}
};


class FNxCodeWindowsEditorModule : public INxCodeWindowsEditorModule
{
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface

};