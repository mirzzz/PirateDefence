
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Editor/UnrealEdEngine.h"
#include "NxCodeWindowsUnrealEdEngine.generated.h"

UCLASS(config=Engine, transient)
class NXCODEWINDOWSEDITOR_API UNxCodeWindowsUnrealEdEngine : public UUnrealEdEngine
{
public:
	GENERATED_BODY()

public:

	//~ Begin UObject Interface.
	~UNxCodeWindowsUnrealEdEngine();

public:
	//~ Begin UEngine Interface.
	virtual void Init(IEngineLoop* InEngineLoop) override;
	virtual void PreExit() override;
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;
	//~ End UEngine Interface.

private:
	UPROPERTY()
	class UNxCodeWindowsManager* CodeWindowsManager;
};
