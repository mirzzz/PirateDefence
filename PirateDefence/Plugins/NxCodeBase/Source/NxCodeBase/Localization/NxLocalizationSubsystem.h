// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Table/NxTableEnum.h"
#include "NxLocalizationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class NXCODEBASE_API UNxLocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UNxLocalizationSubsystem* Get();

public:
	static const FString Code_EN;
	static const FString Code_KR;

public:
	void Initialize(FSubsystemCollectionBase& InCollection) override;

public:
	FString GetCulture();
	void	SetCulture(const FString& InLanguage);

	FText GetFromStringTable(E_StringTable InEnumStringTable, const FString& InKey);
	FText GetFromStringTable(const FName& InTableId, const FString& InKey);
	FText GetText(const FTextKey& InNamespace, FTextKey InKey);
};
