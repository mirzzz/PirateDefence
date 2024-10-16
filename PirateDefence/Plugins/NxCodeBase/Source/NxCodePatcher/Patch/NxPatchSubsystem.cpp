// Fill out your copyright notice in the Description page of Project Settings.


#include "NxPatchSubsystem.h"
#include "IPlatformFilePak.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Kismet/GameplayStatics.h"

#include "NxCodePatcherModule.h"
#include "NxDownloader.h"
#include "NxBaseEnum.h"
#include "NxPatchSettings.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxPatchSubsystem"

UNxPatchSubsystem::UNxPatchSubsystem()
{
}

bool UNxPatchSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	bool bResult = Super::ShouldCreateSubsystem(Outer);
	if (!bResult)
		return false;

	if (CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance() == false)
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 다른 곳에 정의된 재정의 구현이 없는 경우에만 인스턴스를 만듭니다.
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UNxPatchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	NxPrintTxt(LogPatcher, TEXT("[-------------------------------------------------------- PatchSubsystem Initialize]"));

	Super::Initialize(Collection);
}

void UNxPatchSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ShutdownPatching();

	NxPrintTxt(LogPatcher, TEXT("[------------------------------------------------------ PatchSubsystem Deinitialize]"));
}

// Pak 파일 개별 마운트.
bool UNxPatchSubsystem::MountPak(const FString& pakFile, FNxMountInfo& mpi)
{
	if (PakPatchFileMgr.IsValid() == false)
	{
		OriPatchFileMgr = &FPlatformFileManager::Get().GetPlatformFile();
		PakPatchFileMgr = MakeShareable<FPakPlatformFile>(new FPakPlatformFile());

		if (!PakPatchFileMgr->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT("")))
		{
			return false;
		}

		PakPatchFileMgr->InitializeNewAsyncIO();

		FPlatformFileManager::Get().SetPlatformFile(*PakPatchFileMgr.Get());
	}

	FString standardFilename(pakFile);
	FPaths::MakeStandardFilename(standardFilename);
	standardFilename = FPaths::GetPath(standardFilename);

	if (PakPatchFileMgr->Mount(*pakFile, 0, *standardFilename) == false)
	{
		return false;
	}

	static int siMountIndex = 0;
	mpi.Directory  = standardFilename;
	mpi.MountPoint = FString::Printf(_T("/PATCH_%d/"), siMountIndex++);

	// 마운트 설정.
	FPackageName::RegisterMountPoint(mpi.MountPoint, standardFilename);

#if 0 // 디버깅하는 동안 파일과 디렉터리를 덤프할 수 있습니다.
	struct Dump : public IPlatformFile::FDirectoryVisitor
	{
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (bIsDirectory)
			{
				NxPrintLog(LogPatcher, TEXT("Directory: %s"), FilenameOrDirectory);
			}
			else
			{
				NxPrintLog(LogPatcher, TEXT("File: %s"), FilenameOrDirectory);
			}

			//SysOutputf(DownloadManager, bIsDirectory ? _T("Directory: %s") : _T("File: %s"), FilenameOrDirectory);
			return true;
		}
	};
	Dump visitor;

	PakPlatformFile->IterateDirectoryRecursively(*StandardFilename, visitor);
#endif

	if (PakPatchFileMgr.IsValid() == true)
	{
		PakPatchFileMgr.Reset();

		FPlatformFileManager::Get().SetPlatformFile(*OriPatchFileMgr);
	}
	
	return true;
}


bool UNxPatchSubsystem::InitPatching(E_DevelopmentType deployVersion)
{
	// const UNxPatchSettings* PatchSettings = GetDefault<UNxPatchSettings>();
	UNxPatchSettings* patchSettings = GetMutableDefault<UNxPatchSettings>();
	if (patchSettings == nullptr)
		return false;
		
	// 패치할 환경 저장	
	PatchDeployment = NxEnum_GetString(E_DevelopmentType, deployVersion);
	if (PatchDeployment.Equals( "Invalid", ESearchCase::IgnoreCase) )
		return false;
	
	// 버전 URL 경로 가져오기 
	PatchVersionURL = patchSettings->GetPatchVersionURL(deployVersion);
		
	// HTTP module
	FHttpModule& httpModule = FHttpModule::Get();

	// HTTP 요청을 만들고 응답 콜백을 바인딩합니다.
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> request = httpModule.CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &UNxPatchSubsystem::OnPatchVersionResponse);

	// Patch 버전 요청 
	request->SetURL(PatchVersionURL);
	request->SetVerb("GET");
	request->SetHeader(TEXT("User-Agent"), TEXT("KakaoVx"));
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->ProcessRequest();

	return true;
}

void UNxPatchSubsystem::ShutdownPatching()
{
	PatchDeployment.Empty();
	PatchVersionURL.Empty();

	bIsDownloadManifestUpToDate = false;
	bIsPatchingGame = false;

	// 다운로더 제거
	FNxDownloader::Shutdown();
}

void UNxPatchSubsystem::OnPatchVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response == nullptr || bWasSuccessful == false)
	{
		NxPrintLog(LogPatcher, TEXT("Response Failed!, URL Check: %s"), *PatchVersionURL);
		if (PatchWidget)
		{
			FText errorMsg = FText::FromString( FString::Printf(TEXT("Response Failed!, URL Check: %s"), *PatchVersionURL) );
			PatchWidget->ResetProgressBar();
			PatchWidget->SetErrorMessage(errorMsg);
		}
		OnPatchSystemComplete(false);
		return;
	}
	else if (Response->GetResponseCode() != 200)
	{
		NxPrintLog(LogPatcher, TEXT("Response ErrorCode!, %d"), Response->GetResponseCode());
		if (PatchWidget)
		{
			// URL 은 존재하지만, 버전 파일이 없을 경우.
			FText errorMsg = FText::FromString( FString::Printf(TEXT("Response ErrorCode!, %d"), Response->GetResponseCode()) );
			PatchWidget->ResetProgressBar();
			PatchWidget->SetErrorMessage(errorMsg);
		}
		OnPatchSystemComplete(false);
		return;
	}
		
	// 최신 컨텐츠 빌드 정보 받아오기.
	FString contentBuildVersion = Response->GetContentAsString();
	NxPrintLog(LogPatcher, TEXT("Patch Content Build Version : %s"), *contentBuildVersion);

	// 청크 다운로더를 초기화합니다
	NxPrintLog(LogPatcher, TEXT("Get Downloader"));
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetOrCreate();

	// 한번에 처리될 다운로드 수 설정.
	downloader->Initialize(8);	

	// 로컬에 캐싱된 내용 확인
	if (downloader->LoadCachedContent(PatchDeployment))
	{
		NxPrintLog(LogPatcher, TEXT("Cached Build Succeeded"));
	}
	else
	{
		NxPrintLog(LogPatcher, TEXT("Cached Build Skip (No CacheFile)"));
	}

	// 빌드 매니페스트 파일을 업데이트합니다
	TFunction<void(bool bSuccess)> FUNC_ManifestUpdateCompleteCallback = [&](bool bSuccess)
	{
		if (bSuccess)
		{
			NxPrintLog(LogPatcher, TEXT("Mainfest Update Succeded"));
		}
		else
		{
			NxPrintLog(LogPatcher, TEXT("Mainfest Update Failed"));
		}

		OnManifestUpdateComplete(bSuccess);
	};
		
	downloader->UpdateContent(PatchDeployment, contentBuildVersion, FUNC_ManifestUpdateCompleteCallback);
}

// CDN으로 부터 Manifest 을 읽고, 로컬 캐쉬와 비교하여, 다운로드할 정보를 구성완료함.
void UNxPatchSubsystem::OnManifestUpdateComplete(bool bSuccess)
{
	bIsDownloadManifestUpToDate = bSuccess;	

	OnPatchReady.Broadcast(bSuccess);
}

// 게임 패치 시작
bool UNxPatchSubsystem::PatchGame()
{
	// 다운로드 매니페스트를 최신 상태인가?
	if (bIsDownloadManifestUpToDate == false)
	{
		// 매니페스트를 검증하기 위해 서버와 연락하는데 실패하여 패치할 수 없었습니다.
		NxPrintLog(LogPatcher, TEXT("Manifest Update Failed. Can't patch the game"));

		return false;
	}

	NxPrintLog(LogPatcher, TEXT("Starting the game patch process."));

	// 청크 다운로더를 가져옵니다.
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	bIsPatchingGame = true;

	// 매니페스트에서 동기가 필요한 청크를 얻어와서 다운로드를 진행 한다.
	downloader->GetChunkIds("Build", DownloadChunkList);
		
	for (int32 chunkID : DownloadChunkList)
	{
		FNxDownloader::EVxChunkStatus chunkStatus = downloader->GetChunkStatus(chunkID);
		
		NxPrintLog(LogPatcher, TEXT("Chunk %i status: %s"), chunkID, downloader->ChunkStatusToString(chunkStatus) );
	}
		
	if (bIsPatchingLegacy == false)
	{
		TFunction<void(bool)> MountCompleteCallback = [this](bool bSuccess)
		{
			// 패칭 완료됨.
			bIsPatchingGame = false;

			if (bSuccess)
			{
				NxPrintLog(LogPatcher, TEXT("Chunk Mount complete"));

				OnPatchSystemComplete(true);
			}
			else 
			{
				NxPrintLog(LogPatcher, TEXT("Chunk Mount failed"));

				OnPatchSystemComplete(false);
			}
		};

		// 자산에 액세스할 수 있도록 Pak 내용 마운트
		// - MountChunks 함수에 DownloadChunks, BeginLoadingMode 포함됨.
		downloader->MountChunks(DownloadChunkList, MountCompleteCallback);
	}
	else
	{
		// 각 개별 청크 다운로드가 완료될 때 호출되고, 각각에서 다운로드가 성공 또는 실패할 때 메시지를 출력합니다.
		TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess)
		{
			// 지금은 로그에 쓰기만 하면 됩니다.
			if (bSuccess)
			{
				NxPrintLog(LogPatcher, TEXT("Chunk load complete"));
			}
			else
			{
				NxPrintLog(LogPatcher, TEXT("Chunk load failed"));
			}
		};

		downloader->DownloadChunks(DownloadChunkList, DownloadCompleteCallback, 1);

		// 모든 청크가 다운로드되면 발생하며, 로딩 모드를 시작합니다.
		TFunction<void(bool bSuccess)> LoadingModeCallback = [&](bool bSuccess)
		{
			if (bSuccess)
			{
				NxPrintLog(LogPatcher, TEXT("Download complete"));
			}
			else
			{
				NxPrintLog(LogPatcher, TEXT("Download failed"));
			}

			OnLegacyDownloadComplete(bSuccess);
		};

		downloader->BeginLoadingMode(LoadingModeCallback);
	}

	return true;
}


FNxPatchStats UNxPatchSubsystem::GetPatchStatus()
{
	FNxPatchStats retStats;

	// 청크 다운로더의 레퍼런스를 구합니다.
	TSharedPtr<FNxDownloader> downloader = FNxDownloader::Get();
	if(downloader.IsValid() == false)
		return retStats;

	// 로딩 통계 구조체를 구합니다.
	FNxDownloader::FVxStats loadingStats = downloader->GetLoadingStats();

	retStats.FilesDownloaded		= loadingStats.FilesDownloaded;
	retStats.TotalFilesToDownload	= loadingStats.TotalFilesToDownload;

	// 다운로드된 바이트와 다운로드할 바이트를 구합니다. (MB)
	retStats.BytesDownloaded		= (int32)(loadingStats.BytesDownloaded / (1024 * 1024));
	retStats.TotalBytesToDownload	= (int32)(loadingStats.TotalBytesToDownload / (1024 * 1024));

	// 통계를 사용하여 다운로드 및 마운트 퍼센트를 계산합니다.
	retStats.DownloadPercent		= (float)loadingStats.BytesDownloaded / (float)loadingStats.TotalBytesToDownload;

	// 마운트 갯수 확인
	retStats.ChunksMounted			= (int32)(loadingStats.ChunksMounted);
	retStats.TotalChunksToMount		= (int32)(loadingStats.TotalChunksToMount);

	retStats.MountPercent			= (float)loadingStats.ChunksMounted / (float)loadingStats.TotalChunksToMount;

	// 에러 메세지 전달.
	FString LastError = TEXT("");
	if (loadingStats.ErrorTexts.Num() != 0)
	{
		FString LastErrorMsg = loadingStats.ErrorTexts[loadingStats.ErrorTexts.Num()-1];
		LastError = FString::Printf( TEXT("%s - error count:%d"), *LastErrorMsg, loadingStats.ErrorTexts.Num());
	}
	retStats.LastError				= FText::FromString(LastError);

	return retStats;
}


void UNxPatchSubsystem::OnLegacyDownloadComplete(bool bSuccess)
{
	if (!bSuccess)
	{
		NxPrintLog(LogPatcher, TEXT("Download Failed"));
		OnPatchSystemComplete(bSuccess);
		return;
	}

	NxPrintLog(LogPatcher, TEXT("Patch Download Complete"));

	// 청크 다운로더를 가져옵니다.
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	FJsonSerializableArrayInt downloadedChunks;

	for (int32 dhunkID : DownloadChunkList)
	{
		downloadedChunks.Add(dhunkID);
	}

	// 청크를 마운트합니다.
	TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess)
	{
		// 패칭 종료
		bIsPatchingGame = false;
		
		if (bSuccess)
		{
			NxPrintLog(LogPatcher, TEXT("Chunk Mount Complete"));
		}
		else
		{
			NxPrintLog(LogPatcher, TEXT("Chunk Mount Failed"));
		}

		OnPatchSystemComplete(bSuccess);
	};

	downloader->MountChunks(downloadedChunks, MountCompleteCallback);
}


void UNxPatchSubsystem::OnPatchSystemComplete(bool bSuccess)
{
	OnPatchComplete.Broadcast(bSuccess);

	// Single Chunk Support를 위해 필요할 때 해제한다.
	//FVxDownloader::Shutdown();
}

void UNxPatchSubsystem::SetDownloadList(TArray<int32> chunkId)
{
	DownloadChunkList.Append(chunkId);
}


bool UNxPatchSubsystem::IsChunkLoaded(int32 chunkId)
{
	// 매니페스트가 최신 상태인지 확인
	if (!bIsDownloadManifestUpToDate)
		return false;

	// 청크 다운로더 받기
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	// 청크 상태에 대해 청크 다운로더를 쿼리합니다.
	//	- 청크가 마운트된 경우에만 true 를 반환합니다.
	return downloader->GetChunkStatus(chunkId) == FNxDownloader::EVxChunkStatus::Mounted;
}


void UNxPatchSubsystem::AutoPatchBegin()
{
	const UNxPatchSettings* patchSettings = GetDefault<UNxPatchSettings>();
	
	if( patchSettings->PatchEnable == false )
	{
		GotoNextLevel();
		return;
	}

	if (patchSettings->WidgetClass != nullptr && patchSettings->WidgetClass.IsNull() == false)
	{
		TSubclassOf<UUserWidget> patchWidgetClass = patchSettings->WidgetClass.LoadSynchronous();

		// 해당 클래스가 추상클래스가 아닌경우 생성
		if (ensure(patchWidgetClass && patchWidgetClass->HasAnyClassFlags(CLASS_Abstract) == false))
		{
			NxPrintLog(LogBase, TEXT("Create PatchWidget() - [%s]"), *patchWidgetClass->GetName());

			APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);

			PatchWidget = CreateWidget<UNxPatchWidget>(playerController, patchWidgetClass);
			if (PatchWidget)
			{
				PatchWidget->AddToViewport();
				playerController->bShowMouseCursor = true;		// 커서 Show

				PatchWidget->ShowMenu(patchSettings->WidgetMenuEnable);	// UI 의 메뉴 부분 노출 여부
				PatchWidget->ClickPatchInitialize();
			}
		}
	}
	else
	{
		NxPrintTxt(LogPatcher, TEXT("[프로젝트 설정]의 NxPatchSettings의 WidgetClass에 할당된 UI Class가 없음."));

#if WITH_EDITOR
		FString title = TEXT("[PatchSubsystem]");
		FString msg = TEXT("[프로젝트 설정]의 NxPatchSettings의 WidgetClass에 할당된 UI Class가 없음.");
		FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, *msg, *title);
#endif 			
	}

	OnPatchReady.AddDynamic(this, &ThisClass::BindAutoPatchReady);
	OnPatchComplete.AddDynamic(this, &ThisClass::BindAutoPatchComplete);
	OnSingleChunkPatchComplete.AddDynamic(this, &ThisClass::BindAutoSingleChunkPatchComplete);
}

void UNxPatchSubsystem::AutoPatchUpdate(float DeltaTime)
{
	if (PatchWidget && (IsPatchingGame() || IsDownloadingSingleChunks()) )
	{
		PatchWidget->UpdatePatch();
	}
}

// NextMap 으로 자동 전환한다.
bool UNxPatchSubsystem::GotoNextLevel()
{
	if (const UNxPatchSettings* patchSettings = GetDefault<UNxPatchSettings>())
	{
		if(patchSettings->NextMapAfterPatching.IsNull() == true)
			return false ;

		TSoftObjectPtr<UWorld> nextLevel = patchSettings->NextMapAfterPatching;
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, nextLevel);
		return true;
	}
	return false;
}

void UNxPatchSubsystem::BindAutoPatchReady(bool Succeeded)
{
	if (PatchWidget)
	{
		if (Succeeded)
		{
			bool patch = PatchGame();
			if (patch)
			{
				PatchWidget->ShowLayout(true);
				return;
			}
		}
		else
		{
			// Failed...
			FText errorMsg = FText::FromString(TEXT("Patch Failed!"));
			PatchWidget->SetErrorMessage(errorMsg);
			OnPatchSystemComplete(false);
		}
	}
}

void UNxPatchSubsystem::BindAutoPatchComplete(bool Succeeded)
{
	if (PatchWidget)
	{
		if (Succeeded)
		{
			FText okMsg = FText::FromString(TEXT("Patch Success!"));
			PatchWidget->SetMessage(okMsg);
		}
		else
		{
			FText errorMsg = FText::FromString(TEXT("Patch Failed!"));
			PatchWidget->SetErrorMessage(errorMsg);
		}

		// 최종 갱신.
		PatchWidget->UpdatePatch();

		// 콜백 호출.
		if (OnAutoPatchComplete.IsBound())
		{
			OnAutoPatchComplete.Broadcast(Succeeded);			
		}
	}
}

void UNxPatchSubsystem::BindAutoSingleChunkPatchComplete(bool Succeeded)
{
	if (PatchWidget)
	{
		if (Succeeded)
		{
			FText okMsg = FText::FromString(TEXT("Single Chunk Patch Success!"));
			PatchWidget->SetMessage(okMsg);
		}
		else
		{
			FText errorMsg = FText::FromString(TEXT("Single Chunk Patch Failed!"));
			PatchWidget->SetErrorMessage(errorMsg);
		}

		// 최종 갱신.
		PatchWidget->UpdatePatch();

		// 콜백 호출.
		if (OnAutoPatchComplete.IsBound())
		{
			OnAutoPatchComplete.Broadcast(Succeeded);			
		}
	}
}

// Single Chunk Support
bool UNxPatchSubsystem::DownloadSingleChunk(int32 chunkID)
{
	// 다운로드 매니페스트가 최신 버전인지 확인하십시오.
	if (!bIsDownloadManifestUpToDate)
	{
		// 매니페스트의 유효성을 검사하기 위해 서버에 연결할 수 없으므로 패치할 수 없습니다.
		NxPrintLog(LogPatcher, TEXT("Manifest Update Failed. Can't patch the game"));

		return false;
	}

	// 아직 게임을 패치하는 중이라면 개별 다운로드 무시
	if (bIsPatchingGame)
	{
		NxPrintLog(LogPatcher, TEXT("Main game patching underway. Ignoring single chunk downloads."));

		return false;
	}

	// 청크를 여러 번 다운로드하려고 하지 않는지 확인하십시오.
	if (SingleChunkDownloadList.Contains(chunkID))
	{
		NxPrintLog(LogPatcher, TEXT("Chunk: %i already downloading"), chunkID);

		return false;
	}

	// 단일 청크 다운로드 플래그 체크
	bIsDownloadingSingleChunks = true;

	// 다운로드 목록에 청크 추가
	SingleChunkDownloadList.Add(chunkID);

	NxPrintLog(LogPatcher, TEXT("Downloading specific Chunk: %i"), chunkID);

	// 청크 다운로더 받기
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	if( bIsPatchingLegacy == false)
	{
		// pak 마운팅 콜백 람다 함수 준비
		TFunction<void(bool)> MountCompleteCallback = [this](bool bSuccess)
		{
			if (bSuccess)
			{
				NxPrintLog(LogPatcher, TEXT("Single Chunk Mount complete"));

				OnSingleChunkPatchComplete.Broadcast(true);
			}
			else 
			{
				NxPrintLog(LogPatcher, TEXT("Single Mount failed"));

				OnSingleChunkPatchComplete.Broadcast(false);
			}
		};

		// 자산에 액세스할 수 있도록 팩을 탑재
		downloader->MountChunk(chunkID, MountCompleteCallback);
	}
	else
	{
		TFunction<void(bool)> DownloadCompleteCallback = [this](bool bSuccess)
		{
			// 지금은 로그에 쓰기만 하면 됩니다.
			if (bSuccess)
			{
				NxPrintLog(LogPatcher, TEXT("Specific Chunk load complete"));
			}
			else {
				NxPrintLog(LogPatcher, TEXT("Specific Chunk load failed"));
			}
		};

		downloader->DownloadChunk(chunkID, DownloadCompleteCallback, 1);

		// 로드 모드 완료 Lambda를 설정합니다.
		TFunction<void(bool)> LoadingModeCallback = [this](bool bSuccess)
		{
			OnLegacySingleChunkDownloadComplete(bSuccess);
		};

		downloader->BeginLoadingMode(LoadingModeCallback);
	}

	return true;
}

void UNxPatchSubsystem::OnLegacySingleChunkDownloadComplete(bool bSuccess)
{
	bIsDownloadingSingleChunks = false;

	// Pak 다운로드가 성공했습니까?
	if (!bSuccess)
	{
		NxPrintLog(LogPatcher, TEXT("Patch Download Failed"));

		OnSingleChunkPatchComplete.Broadcast(false);
		return;
	}

	NxPrintLog(LogPatcher, TEXT("Patch Download Complete"));

	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	// 다운로드한 청크 목록 작성
	FJsonSerializableArrayInt DownloadedChunks;

	for (int32 ChunkID : SingleChunkDownloadList)
	{
		DownloadedChunks.Add(ChunkID);
	}
	
	TFunction<void(bool)> MountCompleteCallback = [this](bool bSuccess)
	{
		if (bSuccess)
		{
			NxPrintLog(LogPatcher, TEXT("Single Chunk Mount complete"));

			OnSingleChunkPatchComplete.Broadcast(true);
		}
		else {
			NxPrintLog(LogPatcher, TEXT("Single Mount failed"));

			OnSingleChunkPatchComplete.Broadcast(false);
		}
	};

	// 자산에 액세스할 수 있도록 팩을 탑재
	downloader->MountChunks(DownloadedChunks, MountCompleteCallback);
}

#undef LOCTEXT_NAMESPACE