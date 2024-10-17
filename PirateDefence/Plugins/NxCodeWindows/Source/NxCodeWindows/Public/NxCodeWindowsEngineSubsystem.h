
#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "NxCodeWindowsEngineSubsystem.generated.h"

UCLASS()
class NXCODEWINDOWS_API UNxCodeWindowsEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UNxCodeWindowsEngineSubsystem();

public:
 
	// 시스템 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 시스템 해제
	virtual void Deinitialize() override;

public: 

	/**
	 * 모든 뷰포트를 다시 그립니다.
	 * @param	bShouldPresent	Whether we want this frame to be presented
	 */
	virtual void RedrawViewports(bool bShouldPresent = true);

};
