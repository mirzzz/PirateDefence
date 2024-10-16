// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NxBaseEnum.h"
#include "NxPatchSettings.generated.h"


USTRUCT(BlueprintType)
struct FNxPatchingPath
{
	GENERATED_BODY()

public:

	// 패치할 버전파일 CDN 경로
	UPROPERTY(EditAnywhere)
	FString CdnVersionUrls;

	// 패치할 데이터 CDN 경로
	UPROPERTY(EditAnywhere)
	TArray<FString> CdnDataUrls;
};

/**
 * NxCodePatcher Plugin Setting
 *	- Blueprint에서 바로 접근을 지원하지 않으므로, UVxPatchSettginsHelper를 통해 접근한다.
 */
UCLASS(Config = Plugin, DefaultConfig, meta = (DisplayName = "PatchSettings"))
class NXCODEPATCHER_API UNxPatchSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ~UDeveloperSettings
	UNxPatchSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 프로젝트 Category 설정
	virtual FName GetContainerName() const override { return FName("Project"); }	
	virtual FName GetCategoryName() const override { /*return FApp::GetProjectName();*/ return FName("Nx"); }
	virtual FName GetSectionName() const override { return FName("NxCodePatcher"); }


	FString		GetPatchVersionURL(E_DevelopmentType deployType);
	int32		GetPatchDataURL(E_DevelopmentType deployType, TArray<FString>& getArray);


#if WITH_EDITOR
	// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

public:

	// 패치 활성화 여부
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	bool  PatchEnable = false;

	// 요청 실패시 재시도 횟수
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	int32  PatchTryCount = 5;
			
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	E_DevelopmentType  DeployType = E_DevelopmentType::Local;
	
	// 패치 버전 파일
	UPROPERTY(config, EditAnywhere, Category = "Patcher|json")
	FString FILE_SERVER_VERSION = TEXT("ServerVersion.txt");	

	// 서버 리스트 파일
	UPROPERTY(config, EditAnywhere, Category = "Patcher|json")
	FString MANIFEST_SERVER = TEXT("ServerManifest");				

	// 로컬 캐시 리스트 파일
	UPROPERTY(config, EditAnywhere, Category = "Patcher|json")
	FString MANIFEST_CACHED_SERVER = TEXT("CachedServerManifest.json");	

	// 로컬 저장 리스트 파일
	UPROPERTY(config, EditAnywhere, Category = "Patcher|json")
	FString MANIFEST_LOCAL = TEXT("LocalManifest.json");

	// 임베디드 리스트 파일
	UPROPERTY(config, EditAnywhere, Category = "Patcher|json")
	FString MANIFEST_EMBEDDED = TEXT("EmbeddedManifest.json");

	// 패치 경로 
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	TMap<E_DevelopmentType, FNxPatchingPath> PatchPath;

	// 패치 Widget Class
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	TSoftClassPtr<UUserWidget> WidgetClass;

	// 패치 Widget Menu 노출
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	bool WidgetMenuEnable = false;

	// 패치 Next Level
	UPROPERTY(config, EditAnywhere, Category = "Patcher")
	TSoftObjectPtr<UWorld> NextMapAfterPatching;

};
