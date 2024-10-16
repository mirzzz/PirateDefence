// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "NxBaseEnum.h"
#include "NxPatchWidget.h"
#include "NxPatchSubsystem.generated.h"

// patching delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnPatchCompleteDelegate, bool, Succeeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChunkMountedDelegate, int32, ChunkID, bool, Succeeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnAutoPatchCompleteDelegate, bool, Succeeded);


// 진행 피드백에 사용할 수 있는 게임 패치 통계 보고
USTRUCT(BlueprintType)
struct NXCODEPATCHER_API FNxPatchStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 FilesDownloaded = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalFilesToDownload = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BytesDownloaded = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalBytesToDownload = 0;

	// Bytes Percent
	UPROPERTY(BlueprintReadOnly)
	float DownloadPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 ChunksMounted = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalChunksToMount = 0;

	// Mount Percent
	UPROPERTY(BlueprintReadOnly)
	float MountPercent = 0.0f;
		
	UPROPERTY(BlueprintReadOnly)
	FText LastError;
};


USTRUCT(BlueprintType)
struct NXCODEPATCHER_API FNxMountInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FString Directory;
	FString MountPoint;
};

/**
 * Patch 서브 시스템
 *	- Pak데이터를 사용할 경우, SoftClassPtr/SoftObjectPtr을 사용하여 참조 한다. IsSoftObjectReference()
 */
UCLASS()
class NXCODEPATCHER_API UNxPatchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	UNxPatchSubsystem();

	// 서브 시스템 생성 여부 판단.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 해제
	virtual void Deinitialize() override;


	// 마운팅 : pak 안에 들어있는 컨텐츠들이 있는 경로를 생성한 후에 PakFiles에 추가시키는 작업
	bool MountPak(const FString& pakFile, FNxMountInfo& mpi);


	// 패치 다운로드 처리 ----------------------------------------------------
public:
		
	// 전달된 배포 이름(Local, Dev, QA, Live) 으로 패치 시스템을 초기화합니다.
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	bool InitPatching(E_DevelopmentType DeployVersion);
		
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void ShutdownPatching();

	// 패치 추가 정보 필요시 Web에서 받기.
	void OnPatchVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Manifest 버전 완료.
	void OnManifestUpdateComplete(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void SetDownloadList(TArray<int32> chunkId);

	// 게임 패치 프로세스를 시작합니다. 패치 매니페스트가 최신이 아닌 경우 false 를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	bool PatchGame();

	// 진행률 표시줄 등을 채우는 데 사용할 수 있는 패치 상태 보고서를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Patcher")
	FNxPatchStats GetPatchStatus();

	// 청크 다운로드 프로세스가 완료되면 호출됨.
	void OnLegacyDownloadComplete(bool bSuccess);

	// 패치 프로세스가 완료되면 호출됨.
	void OnPatchSystemComplete(bool bSuccess);

	// 주어진 청크가 다운로드되고 마운트되면 true를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Patcher")
	bool IsChunkLoaded(int32 ChunkID);

	// 시스템이 현재 단일 청크를 다운로드 중인지 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Patcher")
	bool IsPatchingGame()
	{
		return bIsPatchingGame;
	}
	 
	// 패치 프로세스가 성공하거나 실패할 때 발생
	UPROPERTY(BlueprintAssignable, Category = "Patcher")
	FOnPatchCompleteDelegate OnPatchReady;

	// 패치 프로세스가 성공하거나 실패할 때 발생
	UPROPERTY(BlueprintAssignable, Category = "Patcher")
	FOnPatchCompleteDelegate OnPatchComplete;

	// Auto Patching
	//-------------------------------------------------------------------------

	// 패치 자동 처리 구문 완료시 호출될 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Patcher")
	FOnAutoPatchCompleteDelegate OnAutoPatchComplete;
	
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void AutoPatchBegin();
	
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void AutoPatchUpdate(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	bool GotoNextLevel();

	// AutoPatch Binding...
	UFUNCTION()
	void BindAutoPatchReady(bool Succeeded);

	UFUNCTION()
	void BindAutoPatchComplete(bool Succeeded);

	UFUNCTION()
	void BindAutoSingleChunkPatchComplete(bool Succeeded);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	UNxPatchWidget* GetPatchWidget() { return PatchWidget; }

protected:

	// DLC 파일을 사용할 수 있는 플랫폼 파일 래퍼.
	TSharedPtr<class FPakPlatformFile> PakPatchFileMgr;
	class IPlatformFile*	   		   OriPatchFileMgr;

	// Version file Url 
	FString PatchVersionURL;

	// Platform Deploy (Local, Dev, QA, Live)
	FString PatchDeployment;
		
	// 다운로드할 청크 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patcher")
	TArray<int32> DownloadChunkList;
		
	// 로컬 매니페스트 파일이 웹사이트에 호스팅된 파일과 같은 최신 업데이트 상태인지 트래킹합니다.
	bool bIsDownloadManifestUpToDate;
		
	// 기본 패치 프로세스가 진행 중인 경우 true 
	bool bIsPatchingGame = false;

	// 다운로드 예전 방법 진행
	bool bIsPatchingLegacy = false;

	// 개별 다운로드 처리 ----------------------------------------------------
public:
	
	// 개별 패치 프로세스가 성공하거나 실패할 때 발생
	UPROPERTY(BlueprintAssignable, Category = "Patcher");
	FOnPatchCompleteDelegate OnSingleChunkPatchComplete;

	// 시스템이 현재 단일 청크를 다운로드 중인지 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Patcher")
	bool IsDownloadingSingleChunks() 
	{ 
		return bIsDownloadingSingleChunks;
	}

	// 다운로드 목록에 개별 청크를 추가하고 로드 및 마운트 프로세스를 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Patcher")
	bool DownloadSingleChunk(int32 ChunkID);

	// 개별 청크 다운로드 프로세스가 완료되면 호출됩니다.
	void OnLegacySingleChunkDownloadComplete(bool bSuccess);

protected:

	// 시스템이 현재 개별 청크를 다운로드 중일 경우 true.
	bool bIsDownloadingSingleChunks = false;

	// 주요 패치 프로세스와 별도로 다운로드할 단일 청크 목록
	TArray<int32> SingleChunkDownloadList;

	// Auto Patching
	//-------------------------------------------------------------------------

	// 패미 자동 진행시 사용할 위젯
	UPROPERTY(Transient)
	TObjectPtr<UNxPatchWidget> PatchWidget = nullptr;
};
