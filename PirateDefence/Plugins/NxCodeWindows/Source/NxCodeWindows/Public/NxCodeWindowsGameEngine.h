
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/GameEngine.h"
#include "NxCodeWindowsGameEngine.generated.h"

UCLASS(config=Engine, transient)
class NXCODEWINDOWS_API UNxCodeWindowsGameEngine : public UGameEngine
{
public:
	GENERATED_BODY()

public:

	//~ Begin UObject Interface.
	~UNxCodeWindowsGameEngine();

public:
	//~ Begin UEngine Interface.
	virtual void Init(IEngineLoop* InEngineLoop) override;
	virtual void PreExit() override;
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;
	//~ End UEngine Interface.

public:
	UPROPERTY()
	class UNxCodeWindowsManager* MultiWindowsManager;
};
