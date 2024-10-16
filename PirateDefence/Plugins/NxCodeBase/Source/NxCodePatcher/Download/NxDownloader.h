// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Containers/Queue.h"
 
template<typename TTask> class FAsyncTask;

class IHttpRequest;
class FDownload;
 
// Pak 파일 단일 정보 저장.
struct NXCODEPATCHER_API FVxPakFileEntry
{
public:

	// pak 파일의 고유 이름(경로 아님, 즉 폴더 없음)
	FString PakName = TEXT("");
	
	// 파일의 최종 크기(바이트)
	uint64  PakBytes = 0;

	// 이 pak 파일의 특정 버전을 나타내는 고유 ID
	// 유효성 검사에 사용될 때(골든 경로에서 수행되지 않지만 요청할 수 있음)
	// "SHA1:"으로 시작하는 경우 SHA1 해시로 간주되고 그렇지 않으면 고유 ID로 간주됩니다.
	FString PakVersion = TEXT("");

	// pak 파일에 할당된 청크 ID
	int32   PakChunkId = -1;

	// pak 파일의 URL ( Cnd 루트 기준, 빌드별 폴더 포함)
	FString PakRelativeUrl = TEXT("");

	// pak 종류에 따라 (Embedded, Build, Course) 다운 패치 결정 함.
	FString	PakType = TEXT("");
};


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlatformChunkInstallMultiDelegate, uint32, bool);


class NXCODEPATCHER_API FNxDownloader : public TSharedFromThis<FNxDownloader>
{
public:
	~FNxDownloader();

	typedef TFunction<void(bool bSuccess)> FNxCallback;

	// static getters
	static TSharedPtr<FNxDownloader> Get();
	static TSharedRef<FNxDownloader> GetChecked();
	static TSharedRef<FNxDownloader> GetOrCreate();
	static void Shutdown();

	// 다운로드 관리자를 초기화합니다 (디스크에서 캐시된 pak 파일 목록 채우기).
	void Initialize(int32 TargetDownloadsInFlightIn);

	// 모든 청크를 마운트 해제하고 진행 중인 다운로드를 취소합니다 (부분 다운로드 유지)
	//	- 한 번만 호출 후, 재사용하지 말고 새로 만듭니다.
	void Finalize();

	// 디스크에서 캐시된 빌드 ID를 로드하려고 합니다(빌드를 업데이트하기 전에 수행하여 작동하지 않을 수 있음)
	bool LoadCachedContent(const FString& DeploymentName);

	// ContentBuild ID설정
	// ContentBuild ID가 변경된 경우 CDN에서 새 BuildManifest를 가져와 로드합니다.
	// BUILD_VERSION 에 따른 ContentBuildID의 호환성 확인을 위해 필요.
	void UpdateContent(const FString& DeploymentNameIn, const FString& ContentVersionIn, const FNxCallback& Callback);

	// 현재 콘텐츠 빌드 ID 가져오기
	inline const FString& GetContentVersion() const { return CurrentContentVersion; }

	// 가장 최근 배포 이름 가져오기
	inline const FString& GetDeploymentName() const { return LastDeploymentName; }

	enum class EVxChunkStatus
	{
		Mounted,		// 청크는 로컬로 캐시되고 RAM에 마운트됩니다.
		Cached,			// 청크가 로컬로 완전히 캐시되지만 마운트되지 않음
		Downloading,	// 청크가 부분적으로 로컬에 캐시되어 탑재되지 않았으며 다운로드가 진행 중입니다.
		Partial,		// 청크가 부분적으로 로컬에 캐시되어 탑재되지 않았으며 다운로드가 진행 중이 아닙니다.
		Remote,			// 로컬 캐싱이 시작되지 않았습니다.
		Unknown,		// 이 청크에는 pak이 포함되어 있지 않으므로 오류 또는 완전히 마운트된 것으로 간주할 수 있습니다.
	};

	static void DumpLoadedChunks();

	// 기록 가능한 문자열로 청크 상태
	static const TCHAR* ChunkStatusToString(EVxChunkStatus Status);

	// Manifest 파일 파싱.
	//static TArray<FVxFilePakEntry> ParseManifestText(const FString& ManifestPath, TMap<FString, FString>* Properties = nullptr);
	static TArray<FVxPakFileEntry> ParseManifestJson(const FString& ManifestPath, TMap<FString, FString>* Properties = nullptr);
	
	// 파일 수정 날자 얻기.
	bool GetFileTimeStamp(FString Path, FDateTime& FileDateTime);

	// 지정된 청크의 현재 상태를 가져옵니다.
	EVxChunkStatus GetChunkStatus(int32 ChunkId) const;

	// 현재 매니페스트의 타입별 청크 ID 목록을 반환.
	void GetChunkIds(FString pakType, TArray<int32>& OutChunkIds) const;

	// 현재 매니페스트의 모든 청크 ID 목록을 반환.
	void GetAllChunkIds(TArray<int32>& OutChunkIds) const;

	// 모든 청크를 다운로드하고 탑재한 다음 콜백을 실행합니다 (여러 MountChunk 호출을 관리하는 편리한 래퍼).
	void MountChunks(const TArray<int32>& ChunkIds, const FNxCallback& Callback);

	// 모든 pak 파일을 다운로드한 다음 순서대로 비동기식으로 마운트합니다 (순서대로 게임 스레드와 비동기식으로).
	void MountChunk(int32 ChunkId, const FNxCallback& Callback);

	// 이 청크에 있는 모든 pak 파일을 다운로드(캐시)한 다음 콜백을 실행합니다(여러 DownloadChunk 호출을 관리하는 편리한 래퍼).
	void DownloadChunks(const TArray<int32>& ChunkIds, const FNxCallback& Callback, int32 Priority = 0);

	// 청크의 모든 pak 파일을 다운로드하지만 마운트하지 않는다. 모든 팩이 캐싱을 완료하면 (성공 또는 실패) 콜백이 시작됩니다.
	// 다운로드는 계속 시도되지만 공간 문제로 인해 실패할 수 있습니다.
	void DownloadChunk(int32 ChunkId, const FNxCallback& Callback, int32 Priority = 0);

	// 현재 다운로드 중이거나 마운트되지 않은 캐시된 파일(디스크에 있음)을 플러시합니다(해당 pak 파일을 마운트 해제하지 않음).
	// 여기에는 전체 및 부분 다운로드가 포함되지만 활성 다운로드는 포함되지 않습니다.
	int FlushCache();

	// 완전히 캐시된 모든 파일(차단)을 읽고 버전 해시를 확인하여 유효성을 검사합니다.
	//	- 일치하지 않는 파일을 자동으로 삭제합니다. 삭제된 파일 수를 반환합니다.
	//	- 이 경우 간단한 업데이트 맵으로 돌아가 NxCodePatcher를 다시 초기화(또는 다시 시작)하는 것이 가장 좋습니다.
	int ValidateCache();

	// 스냅샷 통계 및 로딩 화면 모드로 들어갑니다 (모든 백그라운드 다운로드 일시 중지).
	//	- 모든 논-백그라운드 다운로드가 완료되면 콜백을 실행합니다.
	//	- 프레임이 끝날 때까지 다운로드/마운트가 현재 대기열에 없으면 콜백이 다음 프레임을 실행합니다.	
	void BeginLoadingMode(const FNxCallback& Callback);

	struct FVxStats
	{
		// 다운로드한 pak 파일 수
		int FilesDownloaded = 0;
		int TotalFilesToDownload = 0;

		// 다운로드된 바이트 수
		uint64 BytesDownloaded = 0;
		uint64 TotalBytesToDownload = 0;
		
		// 마운트된 청크 수(청크는 pak의 정렬된 배열입니다)
		int ChunksMounted = 0;
		int TotalChunksToMount = 0;
		
		// 로드가 시작된 UTC 시간(예상 속도)
		FDateTime		LoadingStartTime = FDateTime::MinValue();

		// 에러 메세지 수집
		TArray<FString>	ErrorTexts;
	};

	// 현재 로드 통계를 가져옵니다(일반적으로 로드 모드에 있는 경우에만 유용합니다. BeginLoadingMode 참조).
	inline const FVxStats& GetLoadingStats() const { return LoadingModeStats; }

	// 청크가 마운트될 때마다 호출됩니다(성공 또는 실패). 
	//	- 수동적으로 마운트를 수신하려는 경우에만 이것을 사용하십시오 (그렇지 않으면 MountChunk에서 적절한 요청 콜백을 사용하십시오)
	FOnPlatformChunkInstallMultiDelegate OnChunkMounted;

	// 다운로드 시도가 완료될 때마다 호출됩니다(성공 또는 실패). 
	//	- 수동적으로 듣고 싶은 경우에만 이것을 사용하십시오. 다운로드가 성공할 때까지 재시도합니다.
	TFunction<void(const FString& FileName, const FString& Url, uint64 SizeBytes, const FTimespan& DownloadTime, int32 HttpStatus)> OnDownloadAnalytics;

	// 현재 다운로드 요청 수를 가져와서 다운로드가 진행 중인지 여부를 알 수 있습니다.
	// 다운로드 요청은 FDownload::OnComplete 콜백에서 이 배열에서 제거됩니다.
	inline int32 GetNumDownloadRequests() const { return ArrDownloadRequests.Num(); }

protected:
	
	// Friend class 지정.
	friend class FNxDownloader;
	friend class FNxPatcherPlatformWrapper;
	friend class FNxDownload;

	FNxDownloader();

	static bool CheckFileSha1Hash(const FString& FullPathOnDisk, const FString& Sha1HashStr);

private:
	struct FNxChunk;
	struct FNxPakFile;

	void SetContentBuildVersion(const FString& DeploymentName, const FString& NewContentVersion);

	void LoadManifest(const TArray<FVxPakFileEntry>& PakFiles);

	void TryLoadBuildManifest(int TryNumber);
	void TryDownloadBuildManifest(int TryNumber);

	void WaitForMounts();
	//void SaveLocalManifestText(bool bForce);
	void SaveLocalManifestJson(bool bForce);

	bool UpdateLoadingMode();
	void ComputeLoadingStats();

	void UnmountPakFile(const TSharedRef<FNxPakFile>& PakFile);
	void CancelDownload(const TSharedRef<FNxPakFile>& PakFile, bool bResult);
	void DownloadPakFileInternal(const TSharedRef<FNxPakFile>& PakFile, const FNxCallback& Callback, int32 Priority);

	void MountChunkInternal(FNxChunk& Chunk, const FNxCallback& Callback);
	void DownloadChunkInternal(const FNxChunk& Chunk, const FNxCallback& Callback, int32 Priority);
	void CompleteMountTask(FNxChunk& Chunk);

	bool UpdateMountTasks(float dts);
	void ExecuteNextTick(const FNxCallback& Callback, bool bSuccess);

	void IssueDownloads();

private:

	class  FNxMultiCallback;

	// entry per pak file
	struct FNxPakFile
	{
		FVxPakFileEntry Entry;

		bool	bIsCached  = false;
		bool	bIsMounted = false;
		bool	bIsEmbedded = false;

		// 파일이 다운로드됨에 따라 커집니다. 대상 크기는 Entry.FileSize 를 참조하십시오.
		uint64	SizeOnDisk = 0; 

		// 비동기 다운로드
		int32								Priority = 0;
		TSharedPtr<class FNxDownload>		Download;
		TArray<FNxDownloader::FNxCallback>	PostDownloadCallbacks;
	};

	// 비동기 마운트
	class	FNxPakMountWork;
	typedef FAsyncTask<FNxPakMountWork> FVxMountTask;

	// 청크당 항목
	struct FNxChunk
	{
		int32 ChunkId = -1;
		bool  bIsMounted = false;

		TArray<TSharedRef<FNxPakFile>> PakFiles;

		inline bool IsCached() const
		{
			for (const auto& PakFile : PakFiles)
			{
				if (!PakFile->bIsCached)
				{
					return false;
				}
			}
			return true;
		}

		// 비동기 마운트-
		FVxMountTask* MountTask = nullptr;
	};

private:

	class UNxPatchSubsystem* PatchSystem;

	// pak 파일을 사용할 수 있는 플랫폼 파일 래퍼.
	TSharedPtr<class FPakPlatformFile>	PakDownloaderFileMgr;
	class IPlatformFile*				OriDownloaderFileMgr;

	// 로딩 화면 모드의 누적 통계
	FVxStats			LoadingModeStats;
	TArray<FNxCallback> PostLoadCallbacks;
	int32				LoadingCompleteLatch = 0;

	FNxCallback UpdateBuildCallback;
	
	// 디스크에 pak 파일을 저장할 폴더
	FString CacheFolder;

	// 빌드와 함께 제공되는 일부 청크를 찾을 수 있는 콘텐츠 폴더
	FString EmbeddedFolder;

	// 특정 버전 및 URL 경로 빌드
	FString LastDeploymentName;
	FString CurrentContentVersion;
	TArray<FString> BuildDataUrls;

	// 청크 레코드에 대한 청크 ID
	TMap<int32, TSharedRef<FNxChunk>> MapChunks;

	// pak 파일 이름, pak 파일 레코드 표현
	TMap<FString, TSharedRef<FNxPakFile>> MapPakFiles;

	// 빌드에 포함된 pak 파일 (불변)
	TMap<FString, FVxPakFileEntry> MapEmbeddedPaks;

	// 매니페스트를 재 저장해야 하는지 여부
	bool bNeedsManifestSave = false;

	// 메인 스레드의 프레임별 마운트 티커 핸들
	FTSTicker::FDelegateHandle MountTicker;

	// 매니페스트 다운로드 요청
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ManifestRequest;

	// 동시에 허용할 수 있는 최대 다운로드 수
	int32 TargetDownloadsInFlight = 1;

	// 요청된 pak 파일 목록
	TArray<TSharedRef<FNxPakFile>> ArrDownloadRequests;
};

