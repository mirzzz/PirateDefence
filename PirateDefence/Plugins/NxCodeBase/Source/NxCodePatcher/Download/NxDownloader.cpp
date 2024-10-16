// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxDownloader.h"

#include "CoreMinimal.h"
#include "Templates/UniquePtr.h"
#include "Containers/Ticker.h"
#include "Async/AsyncWork.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProperties.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/CoreDelegates.h"
#include "Misc/SecureHash.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/ConfigCacheIni.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "IPlatformFilePak.h"

#include "NxCodePatcherModule.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "NxPatchSubsystem.h"
#include "HAL/FileManagerGeneric.h"
#include "NxDownload.h"

#include "NxBaseEnum.h"
#include "NxBaseLog.h"
#include "NxPatchSettings.h"

#define LOCTEXT_NAMESPACE "NxDownaloder"

//static const FString MANIFEST_SERVER			= TEXT("ServerManifest");				// json	
//static const FString MANIFEST_CACHED_SERVER	= TEXT("CachedServerManifest.json");	// json
//static const FString MANIFEST_LOCAL			= TEXT("LocalManifest.json");		// json
//static const FString MANIFEST_EMBEDDED		= TEXT("EmbeddedManifest.json");	// json

// FileVersion 1 관련 Key
static const FString PATCH_FILEVERSION		= TEXT("FileVersion");
static const FString PATCH_CONTENT_PLATFORM	= TEXT("ContentPlatform");
static const FString PATCH_CONTENT_VERSION	= TEXT("ContentVersion");
static const FString PATCH_CONTENT_PAKCOUNT	= TEXT("PakCount");
static const FString PATCH_CONTENT_PAKFILES	= TEXT("PakFiles");
static const FString PATCH_CONTENT_DESC		= TEXT("Description");

// PakEntry
static const FString PAKENTRY_NAME			= TEXT("PakName");
static const FString PAKENTRY_BYTES			= TEXT("PakBytes");
static const FString PAKENTRY_VERSION		= TEXT("PakVersion");
static const FString PAKENTRY_TYPE			= TEXT("PakType");
static const FString PAKENTRY_CHUNKID		= TEXT("PakChunkId");
static const FString PAKENTRY_RELATIVERUL	= TEXT("PakRelativeUrl");


//static 
bool FNxDownloader::CheckFileSha1Hash(const FString& FullPathOnDisk, const FString& Sha1HashStr)
{
	IFileHandle* FilePtr = IPlatformFile::GetPlatformPhysical().OpenRead(*FullPathOnDisk);
	if (FilePtr == nullptr)
	{
		NxPrintError(LogPatcher, TEXT("Unable to open %s for hash verify."), *FullPathOnDisk);
		return false;
	}

	// create a SHA1 reader
	FSHA1 HashContext;

	// 메모리 최고 수위 표시를 너무 많이 높이는 것을 방지하기 위해 64K 청크로 읽습니다.
	{
		static const int64 FILE_BUFFER_SIZE = 64 * 1024;
		uint8 Buffer[FILE_BUFFER_SIZE];
		int64 FileSize = FilePtr->Size();
		for (int64 Pointer = 0; Pointer<FileSize;)
		{
			// 이번 반복으로 읽을 바이트 수
			int64 SizeToRead = FileSize - Pointer;
			if (SizeToRead > FILE_BUFFER_SIZE)
			{
				SizeToRead = FILE_BUFFER_SIZE;
			}

			// 바이트 읽기
			if (!FilePtr->Read(Buffer, SizeToRead))
			{
				NxPrintError(LogPatcher, TEXT("Read error while validating '%s' at offset %lld."), *FullPathOnDisk, Pointer);

				// 파일 닫기
				delete FilePtr;
				return false;
			}
			Pointer += SizeToRead;

			// 해시 업데이트
			HashContext.Update(Buffer, SizeToRead);
		}

		// 파일 완료
		delete FilePtr;
	}

	// 청크 닫기
	HashContext.Final();
	uint8 FinalHash[FSHA1::DigestSize];
	HashContext.GetHash(FinalHash);

	// 방금 받는 해시 문자열을 빌드.
	FString LocalHashStr = TEXT("SHA1:");
	for (int Idx = 0; Idx < 20; Idx++)
	{
		LocalHashStr += FString::Printf(TEXT("%02X"), FinalHash[Idx]);
	}
	return Sha1HashStr == LocalHashStr;
}

static bool WriteStringAsUtf8TextFile(const FString& FileText, const FString& FilePath)
{
	// UTF8 변환
	FTCHARToUTF8 PakFileUtf8(*FileText);

	// 파일 쓰기
	bool bSuccess = false;

	// Module Check
#if PLATFORM_WINDOWS	

#elif PLATFORM_ANDROID

#elif PLATFORM_IOS

#endif

	IPlatformFile& platformFile = IPlatformFile::GetPlatformPhysical();
	IFileHandle*   manifestFile = platformFile.OpenWrite(*FilePath, true);
	if (manifestFile != nullptr)
	{
		// 기록
		if (manifestFile->Write(reinterpret_cast<const uint8*>(PakFileUtf8.Get()), PakFileUtf8.Length()))
		{
			NxPrintLog(LogPatcher, TEXT("Wrote to %s"), *FilePath);
			bSuccess = true;
		}
		else
		{
			NxPrintError(LogPatcher, TEXT("Write error writing to %s"), *FilePath);
		}

		// handle 종료
		delete manifestFile;
	}
	else
	{
		NxPrintError(LogPatcher, TEXT("Unable open %s for writing."), *FilePath);
	}
	return bSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////

class FNxDownloader::FNxMultiCallback
{
public:
	FNxMultiCallback(const FNxCallback& Callback) : OuterCallback(Callback)
	{
		IndividualCb = [this](bool bSuccess) {
			// 통계 업데이트
			--NumPending;
			if (bSuccess)
				++NumSucceeded;
			else
				++NumFailed;

			// 우리가 마지막이라면 외부 콜백을 트리거합니다.
			if (NumPending <= 0)
			{
				check(NumPending == 0);
				if (OuterCallback)
				{
					OuterCallback(NumFailed <= 0);
				}

				// 종료
				delete this;
			}
		};
	}

	inline const FNxCallback& AddPending()
	{ 
		++NumPending;
		return IndividualCb;
	}

	inline int GetNumPending() const { return NumPending; }

	void Abort()
	{
		check(NumPending == 0);
		delete this;
	}

private:
	~FNxMultiCallback() {}

	int NumPending = 0;
	int NumSucceeded = 0;
	int NumFailed = 0;
	FNxCallback IndividualCb;
	FNxCallback OuterCallback;
};

////////////////////////////////////////////////////////////////////////////////////////////

class FNxDownloader::FNxPakMountWork : public FNonAbandonableTask
{
public:
	friend class FAsyncTask<FNxPakMountWork>;

	void DoWork()
	{
		// pak 파일 마운트 시도
		if (FCoreDelegates::MountPak.IsBound())
		{
			uint32 pakReadOrder = PakFiles.Num();

			for (const TSharedRef<FNxPakFile>& pakFile : PakFiles)
			{
				FString fullPathOnDisk = (pakFile->bIsEmbedded ? EmbeddedFolder : CacheFolder) / pakFile->Entry.PakName;
				NxPrintLog(LogPatcher, TEXT("MountPaK Path : %s. IsEmbeded :%d"), *fullPathOnDisk, pakFile->bIsEmbedded);

				IPakFile* mountedPak = FCoreDelegates::MountPak.Execute(fullPathOnDisk, pakReadOrder);

#if !UE_BUILD_SHIPPING
				if (!mountedPak)
				{
					// pak 시스템이 이해하지 못하는 샌드박스 시스템 때문에 실패할 수 있습니다.
					FString sandboxedPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*fullPathOnDisk);
					mountedPak = FCoreDelegates::MountPak.Execute(sandboxedPath, pakReadOrder);
				}
#endif

				if (mountedPak)
				{
					// 이 pak 파일을 성공적으로 마운트했음을 기록합니다.
					MountedPakFiles.Add(pakFile);

					--pakReadOrder;
				}
				else
				{
					NxPrintError(LogPatcher, TEXT("Unable to mount %s from chunk %d (mount operation failed)"), *fullPathOnDisk, ChunkId);
				}
			}
		}
		else
		{
			NxPrintError(LogPatcher, TEXT("Unable to mount chunk %d (no FCoreDelegates::MountPak bound)"), ChunkId);
		}
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FNxPakMountWork, STATGROUP_ThreadPoolAsyncTasks);
	}

public: // inputs

	int32 ChunkId;

	// 디스크에 pak 파일을 저장할 폴더
	FString CacheFolder;
	FString EmbeddedFolder;

	// 마운트 할 순서
	TArray<TSharedRef<FNxPakFile>> PakFiles;

	// 마운트 완료 후 콜백
	TArray<FNxCallback> PostMountCallbacks;

public: // results

	// 성공적으로 마운트된 파일
	TArray<TSharedRef<FNxPakFile>> MountedPakFiles;
};

////////////////////////////////////////////////////////////////////////////////////////////

FNxDownloader::FNxDownloader()
{
}

FNxDownloader::~FNxDownloader()
{
	check(MapPakFiles.Num() <= 0);
}

/*
// 파일 파싱
TArray<FVxFilePakEntry> FVxDownloader::ParseManifestText(const FString& ManifestPath, TMap<FString,FString>* Properties)
{
	int32 ExpectedEntries = -1;
	TArray<FVxFilePakEntry> Entries;
	IFileHandle* ManifestFile = IPlatformFile::GetPlatformPhysical().OpenRead(*ManifestPath);
	if (ManifestFile != nullptr)
	{
		int64 FileSize = ManifestFile->Size();
		if (FileSize > 0)
		{
			NxPrintLog(LogPatcher, TEXT("Found manifest at %s"), *ManifestPath);

			// read the whole file into a buffer (expecting UTF-8 so null terminate)
			// little extra since we're forcing null term in places outside bounds of a field
			char* FileBuffer = new char[FileSize + 8]; 
			if (ManifestFile->Read((uint8*)FileBuffer, FileSize))
			{
				FileBuffer[FileSize] = '\0';

				// make buffers for stuff read from each line
				char NameBuffer[512] = { 0 };
				uint64 FinalFileLen = 0;
				char VersionBuffer[512] = { 0 };
				int32 ChunkId = -1;
				char RelativeUrl[2048] = { 0 };

				// line end
				int LineNum = 0;
				int NextLineStart = 0;
				while (NextLineStart < FileSize)
				{
					int LineStart = NextLineStart;
					++LineNum;

					// 줄의 끝을 검색
					int LineEnd = LineStart;
					while (LineEnd < FileSize && FileBuffer[LineEnd] != '\n' && FileBuffer[LineEnd] != '\r')
					{
						++LineEnd;
					}

					// 다음 줄의 끝을 검색
					NextLineStart = LineEnd + 1;
					while (NextLineStart < FileSize && (FileBuffer[NextLineStart] == '\n' || FileBuffer[NextLineStart] == '\r'))
					{
						++NextLineStart;
					}

					// see if this is a property
					if (FileBuffer[LineStart] == '$')
					{
						// parse the line
						char* NameStart = &FileBuffer[LineStart+1];
						char* NameEnd = FCStringAnsi::Strstr(NameStart, " = ");
						if (NameEnd != nullptr)
						{
							char* ValueStart = NameEnd + 3;
							char* ValueEnd = &FileBuffer[LineEnd];
							*NameEnd = '\0';
							*ValueEnd = '\0';

							FString Name = FUTF8ToTCHAR(NameStart, NameEnd-NameStart + 1).Get();
							FString Value = FUTF8ToTCHAR(ValueStart, ValueEnd-ValueStart + 1).Get();
							if (Properties != nullptr)
							{
								Properties->Add(Name,Value);
							}

							if (Name == TEXT("NUM_ENTRIES"))
							{
								ExpectedEntries = FCString::Atoi(*Value);
							}
						}
						continue;
					}

					// parse the line
#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
					if (!ensure(sscanf_s(&FileBuffer[LineStart], "%511[^\t]\t%llu\t%511[^\t]\t%d\t%2047[^\r\n]", 
						NameBuffer, (int)sizeof(NameBuffer), 
						&FinalFileLen, 
						VersionBuffer, (int)sizeof(VersionBuffer), 
						&ChunkId, 
						RelativeUrl, (int)sizeof(RelativeUrl)
					) == 5))
#else
					if (!ensure(sscanf(&FileBuffer[LineStart], "%511[^\t]\t%llu\t%511[^\t]\t%d\t%2047[^\r\n]", 
						NameBuffer, 
						&FinalFileLen, 
						VersionBuffer, 
						&ChunkId, 
						RelativeUrl
					) == 5))
#endif
					{
						NxPrintError(LogPatcher, TEXT("Manifest parse error at %s:%d"), *ManifestPath, LineNum);
						continue;
					}

					// add a new pak file entry
					FVxFilePakEntry Entry;
					Entry.PakName = UTF8_TO_TCHAR(NameBuffer);
					Entry.PakSizeBytes = FinalFileLen;
					Entry.PakVersion = UTF8_TO_TCHAR(VersionBuffer);
					if (ChunkId >= 0)
					{
						Entry.PakChunkId = ChunkId;
						Entry.PakRelativeUrl = UTF8_TO_TCHAR(RelativeUrl);
					}
					Entries.Add(Entry);
				}

				// all done
				delete[] FileBuffer;
			}
			else
			{
				NxPrintError(LogPatcher, TEXT("Read error loading manifest at %s"), *ManifestPath);
			}
		}
		else
		{
			NxPrintLog(LogPatcher, TEXT("Empty manifest found at %s"), *ManifestPath);
		}
		
		// close the file
		delete ManifestFile;
	}
	else
	{
		NxPrintLog(LogPatcher, TEXT("No manifest found at %s"), *ManifestPath);
	}

	if (ExpectedEntries >= 0 && ExpectedEntries != Entries.Num())
	{
		NxPrintError(LogPatcher, TEXT("Corrupt manifest at %s (expected %d entries, got %d)"), *ManifestPath, ExpectedEntries, Entries.Num());
		Entries.Empty();
		if (Properties != nullptr)
		{
			Properties->Empty();
		}
	}

	return Entries;
}
*/

// 파일 파싱
TArray<FVxPakFileEntry> FNxDownloader::ParseManifestJson(const FString& manifestPath, TMap<FString, FString>* properties)
{
	int32 expectedEntries = -1;

	TArray<FVxPakFileEntry> entries;
	
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*manifestPath) == true)
	{
		FString fileContent = "";
		FFileHelper::LoadFileToString(fileContent, *manifestPath);

		TSharedRef<TJsonReader<TCHAR>>	reader = TJsonReaderFactory<TCHAR>::Create(*fileContent);
		TSharedPtr<FJsonObject>			jsonObj = MakeShareable(new FJsonObject());

		if (FJsonSerializer::Deserialize(reader, jsonObj))
		{
			FString fileVer		= jsonObj->GetStringField(PATCH_FILEVERSION);
			FString platform	= jsonObj->GetStringField(PATCH_CONTENT_PLATFORM);
			FString version		= jsonObj->GetStringField(PATCH_CONTENT_VERSION);
			FString description = jsonObj->GetStringField(PATCH_CONTENT_DESC);
					
			if (properties != nullptr)
			{
				properties->Add(PATCH_FILEVERSION,		fileVer);
				properties->Add(PATCH_CONTENT_PLATFORM,	platform);
				properties->Add(PATCH_CONTENT_VERSION,	version);
				properties->Add(PATCH_CONTENT_DESC,		description);
			}

			expectedEntries = jsonObj->GetIntegerField(PATCH_CONTENT_PAKCOUNT);

			const TArray<TSharedPtr<FJsonValue>>* patchPaks;

			if (jsonObj->TryGetArrayField(PATCH_CONTENT_PAKFILES, patchPaks))
			{
				for (int prev = 0; prev < patchPaks->Num(); ++prev)
				{
					FVxPakFileEntry entry;

					TSharedPtr<FJsonValue>  jsonValue = (*patchPaks)[prev];
					TSharedPtr<FJsonObject> jsonPak  = jsonValue->AsObject();

					entry.PakName		= jsonPak->GetStringField(PAKENTRY_NAME);
					entry.PakBytes		= jsonPak->GetIntegerField(PAKENTRY_BYTES);
					entry.PakVersion	= jsonPak->GetStringField(PAKENTRY_VERSION);
					entry.PakType		= jsonPak->GetStringField(PAKENTRY_TYPE);
					
					int32 chunkId = -1;
					chunkId = jsonPak->GetIntegerField(PAKENTRY_CHUNKID);
					if (chunkId >= 0)
					{
						entry.PakChunkId = chunkId;
						entry.PakRelativeUrl = jsonPak->GetStringField(PAKENTRY_RELATIVERUL);
					}
					entries.Add(entry);
				}
			}
			else
			{
				NxPrintError(LogPatcher, TEXT("PakFiles Is Empty!"));
			}
		}

		// Pak 파일갯수 비교를 통해 파일 유효성 검사.
		if (expectedEntries >= 0 && expectedEntries != entries.Num())
		{
			entries.Empty();
			if (properties != nullptr)
			{
				properties->Empty();
			}

			NxPrintError(LogPatcher, TEXT("Corrupt manifest at %s (expected %d entries, got %d)"), *manifestPath, expectedEntries, entries.Num());
		}
	}

	return entries;
}

// 파일 수정 날자 
bool FNxDownloader::GetFileTimeStamp(FString path, FDateTime& fileDateTime)
{
	FFileManagerGeneric fm;
	
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path) == true)
	{
		const TCHAR* filePath = *path;

		fileDateTime = fm.GetTimeStamp(filePath);

		return true;
	}
	return false;
}

void FNxDownloader::Initialize(int32 targetDownloadsInFlightIn)
{
	check(MapPakFiles.Num() == 0); 
	
	// pak 플랫폼이 유효한지 확인
	if (PakDownloaderFileMgr.IsValid() == false)
	{
		// 현재 플랫폼 파일 관리자 저장
		OriDownloaderFileMgr = &FPlatformFileManager::Get().GetPlatformFile();

		PakDownloaderFileMgr = MakeShareable<FPakPlatformFile>(new FPakPlatformFile());

		// pak 플랫폼 파일을 초기화합니다. 여기에서 MountPak 대리자가 설정됩니다.
		PakDownloaderFileMgr->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT(""));

		// 초기화된 Pak 플랫폼 파일 설정
		FPlatformFileManager::Get().SetPlatformFile(*PakDownloaderFileMgr.Get());
	}
		
	NxPrintDisplay(LogPatcher, TEXT("Initializing with TargetDownloadsInFlightIn='%d'"), targetDownloadsInFlightIn);

	// PatchSystem 등록
	FWorldContext* worldContext = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
	UGameInstance* gameInst = UGameplayStatics::GetGameInstance(worldContext->World());

	PatchSystem = gameInst->GetSubsystem<UNxPatchSubsystem>();
	if (PatchSystem == nullptr)
	{
		NxPrintError(LogPatcher, TEXT("PatchSystem is Not Exist!"));
		return;
	}
	
	// PersistentDownloadDir/Pakcache 폴더 생성
    FPlatformMisc::AddAdditionalRootDirectory(FPaths::Combine(*FPaths::ProjectPersistentDownloadDir(), TEXT("pakcache")));

#if UE_BUILD_DEBUG
	TArray<FString> RootDirectories = FPlatformMisc::GetAdditionalRootDirectories();
	for (auto dir : RootDirectories)
	{
		NxPrintDisplay(LogPatcher, TEXT("RootDirectory : '%s' "), *dir);
	}
#endif 
	
	// 동시에 처리할 갯수 설정
	TargetDownloadsInFlight = targetDownloadsInFlightIn;
	check(TargetDownloadsInFlight >= 1);

	// 캐싱 및 저장 폴더 설정
	CacheFolder		= FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/");
	EmbeddedFolder	= FPaths::ProjectContentDir() / TEXT("EmbeddedPaks/");

	// 캐시 폴더가 있는지 확인
	IFileManager& fileManager = IFileManager::Get();
	if (!fileManager.MakeDirectory(*CacheFolder, true))
	{
		NxPrintError(LogPatcher, TEXT("Unable to create cache folder at '%s'"), *CacheFolder);
	}

	// Enbedded 폴더 파일 확인
	const UNxPatchSettings* settings = GetDefault<UNxPatchSettings>();
	MapEmbeddedPaks.Empty();
	for (const FVxPakFileEntry& Entry : ParseManifestJson(EmbeddedFolder / settings->MANIFEST_EMBEDDED))
	{
		MapEmbeddedPaks.Add(Entry.PakName, Entry);
	}

	// Cached 폴더 파일 항목 임시 보관
	TArray<FString> strayFiles;
	fileManager.FindFiles(strayFiles, *CacheFolder, TEXT("*.pak"));

	// LocalManifest를 로드하여 디스크에 있는 내용을 확인합니다.
	TArray<FVxPakFileEntry> localManifest = ParseManifestJson(CacheFolder / settings->MANIFEST_LOCAL);
	if (localManifest.Num() > 0)
	{
		// 로컬 캐시의 각 항목에 대해 PakFileInfo에 항목 만들기 
		for (const FVxPakFileEntry& entry : localManifest)
		{
			// 새 파일 정보 만들기
			TSharedRef<FNxPakFile> fileInfo = MakeShared<FNxPakFile>();

			// 입력 필드 위에 복사 
			fileInfo->Entry = entry;

			// 로컬 Pak 파일 크기 확인
			FString localPath   = CacheFolder / entry.PakName;
			int64 sizeOnLocalDisk = fileManager.FileSize(*localPath);
			if (sizeOnLocalDisk > 0)
			{
				fileInfo->SizeOnDisk = (uint64)sizeOnLocalDisk;

				// 로컬 Disk Pak 크기와 Manifest 크기 정보가 다른 경우.
				if (fileInfo->SizeOnDisk > entry.PakBytes)
				{
					// LocalManifest 에 쓰여 있는 크기보가 실제크기가 더 큰 경우, LocalManifest 재 저장
					NxPrintWarning(LogPatcher, TEXT("Found '%s' on disk with size larger than LocalManifest indicates"), *localPath);					

					bNeedsManifestSave = true;
					continue;
				}

				// 완전히 Cache 된 파일인지 확인.
				if (fileInfo->SizeOnDisk == entry.PakBytes)
				{
					// 이미 다운로드 Cache가 완료 되었음.
					fileInfo->bIsCached = true;
				}

				// 정보 추가
				MapPakFiles.Add(entry.PakName, fileInfo);
			}
			else
			{
				// LocalManifest 에서 이것을 제거하고 다시 저장하십시오 
				// - 파일 다운로드가 성공적으로 시작되기 전에 충돌했을 수 있음
				NxPrintLog(LogPatcher, TEXT("'%s' appears in LocalManifest but is not on disk (not necessarily a problem)"), *localPath);

				bNeedsManifestSave = true;
			}

			// StrayFiles 에서 제거
			strayFiles.RemoveSingle(entry.PakName);
		}
	}

	// 로컬 매니페스트에 없는 모든 항목을 삭제 (파일 삭제)
	for (FString orphan : strayFiles)
	{
		bNeedsManifestSave = true;

		FString fullPathOnDisk = CacheFolder / orphan;

		NxPrintLog(LogPatcher, TEXT("Deleting orphaned file '%s'"), *fullPathOnDisk);
		if (!ensure(fileManager.Delete(*fullPathOnDisk)))
		{
			// 오류 기록 (ToDo)
			NxPrintError(LogPatcher, TEXT("Unable to delete '%s'"), *fullPathOnDisk);
		}
	}

	// 로컬 매니페스트 다시 저장
	SaveLocalManifestJson(false);
}

bool FNxDownloader::LoadCachedContent(const FString& deploymentName)
{
	// ContentBuildVersion이 있을 경우, 캐시된 매니페스트 다시 채우기 시도
	const UNxPatchSettings* settings = GetDefault<UNxPatchSettings>();

	TMap<FString, FString> cachedManifestProps;
	TArray<FVxPakFileEntry> cachedManifest = ParseManifestJson(CacheFolder / settings->MANIFEST_CACHED_SERVER, &cachedManifestProps);
	const FString* contentVersion = cachedManifestProps.Find(PATCH_CONTENT_VERSION);
	if (contentVersion == nullptr || contentVersion->IsEmpty())
	{
		return false;
	}

	SetContentBuildVersion(deploymentName, *contentVersion);
	LoadManifest(cachedManifest);
	return true;
}

// DeploymentName : Local, Dev, QA, Live
void FNxDownloader::SetContentBuildVersion(const FString& deploymentName, const FString& newContentVersion)
{
	// 콘텐츠 빌드 ID 저장
	CurrentContentVersion	= newContentVersion;
	LastDeploymentName		= deploymentName;

	NxPrintDisplay(LogPatcher, TEXT("Deployment = %s, ContentVersion = %s"), *deploymentName, *CurrentContentVersion);

	// 배포 구성에서 CDN URL 읽기
	TArray<FString> cdnDataUrls;
	
	// Step1. PatchSettings 에서 정보 가져오기
	UNxPatchSettings* PatchSettings = GetMutableDefault<UNxPatchSettings>();
	if (PatchSettings != nullptr)
	{
		E_DevelopmentType deployType = NxEnum_GetEnum(E_DevelopmentType, deploymentName);
		PatchSettings->GetPatchDataURL(deployType, cdnDataUrls);
	}

	if(cdnDataUrls.Num() == 0)
	{
		NxPrintWarning(LogPatcher, TEXT("No CDN base URLs PatchSetting or ConfigXml in [%s]. Chunk downloading will only be able to use embedded cache."), *deploymentName);
	}

	// BaseURL 구성
	BuildDataUrls.Empty();

	for (int32 i=0, n=cdnDataUrls.Num(); i<n ; ++i)
	{
		const FString& BaseUrl = cdnDataUrls[i];
		check(!BaseUrl.IsEmpty());

		FString BuildUrl = BaseUrl;
		NxPrintDisplay(LogPatcher, TEXT("ContentDataUrl[%d] = %s"), i, *BuildUrl);

		BuildDataUrls.Add(BuildUrl);
	}
}

void FNxDownloader::UpdateContent(const FString& deploymentNameIn, const FString& contentVersionIn, const FNxCallback& callback)
{
	check(!contentVersionIn.IsEmpty());

	// 빌드 환경이 같고, 컨텐츠 버전이 변경되지 않은 경우 수행할 작업이 없습니다.
	if (CurrentContentVersion == contentVersionIn && LastDeploymentName == deploymentNameIn)
	{
		ExecuteNextTick(callback, true);
		return;
	}

	// 다운받은 URL 설정
	SetContentBuildVersion(deploymentNameIn, contentVersionIn);

	// 중복된 UpdateBuild 호출이 허용되지 않으며 콜백이 필요합니다.
	check(!UpdateBuildCallback);
	check(callback);

	UpdateBuildCallback = callback;

	// 로드/다운로드 프로세스 시작
	TryLoadBuildManifest(0);
}

void FNxDownloader::Finalize()
{
	NxPrintDisplay(LogPatcher, TEXT("Finalizing."));

	// 모든 마운트가 완료될 때까지 기다립니다.
	WaitForMounts();

	// 마운트 작업 업데이트(콜백 대기)
	ensure(UpdateMountTasks(0.0f) == false);

	// 모든 다운로드 취소
	for (const auto& It : MapPakFiles)
	{
		const TSharedRef<FNxPakFile>& file = It.Value;

		if (file->Download.IsValid())
		{
			CancelDownload(file, false);
		}
	}

	// 마운트된 모든 청크를 마운트 해제(최선의 노력)
	for (const auto& It : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = It.Value;

		if (chunk->bIsMounted)
		{
			// 팩을 마운트 해제 (역순으로)
			for (int32 i=chunk->PakFiles.Num()-1;i >= 0; --i)
			{
				const TSharedRef<FNxPakFile>& pakFile = chunk->PakFiles[i];
				UnmountPakFile(pakFile);
			}

			// clear the flag
			chunk->bIsMounted = false;
		}
	}

	// pak 파일 및 청크 지우기
	MapPakFiles.Empty();
	MapChunks.Empty();

	// 보류 중인 매니페스트 요청 취소
	if (ManifestRequest.IsValid())
	{
		ManifestRequest->CancelRequest();
		ManifestRequest.Reset();
	}

	// 모든 로딩 처리가 사실상 완료됨
	if (PostLoadCallbacks.Num() > 0)
	{
		TArray<FNxCallback> callbacks = MoveTemp(PostLoadCallbacks);

		PostLoadCallbacks.Empty();
		for (const auto& Callback : callbacks)
		{
			ExecuteNextTick(Callback, false);
		}
	}

	// 업데이트도 사실상 완료되었습니다.
	if (UpdateBuildCallback)
	{
		FNxCallback callback = MoveTemp(UpdateBuildCallback);
		ExecuteNextTick(callback, false);
	}

	// 콘텐츠 빌드 ID 지우기
	CurrentContentVersion.Empty();

	if (PakDownloaderFileMgr.IsValid() == true)
	{
		PakDownloaderFileMgr.Reset();

		// pak 파일 업데이트를 수행한 후 원래 플랫폼 파일을 다시 설정합니다.
		FPlatformFileManager::Get().SetPlatformFile(*OriDownloaderFileMgr);
	}
}

/*
void FVxDownloader::SaveLocalManifestText(bool bForce)
{
	if (bForce || bNeedsManifestSave)
	{
		// 필요할 경우 LocalManifest 파일 재저장
		int32 NumEntries = 0;
		for (const auto& It : MapPakFiles)
		{
			if (!It.Value->bIsEmbedded)
			{
				if (It.Value->SizeOnDisk > 0 || It.Value->Download.IsValid())
				{
					++NumEntries;
				}
			}
		}

		FString PakFileText = FString::Printf(TEXT("$NUM_ENTRIES = %d\n"), NumEntries);
		for (const auto& It : MapPakFiles)
		{
			if (!It.Value->bIsEmbedded)
			{
				if (It.Value->SizeOnDisk > 0 || It.Value->Download.IsValid())
				{
					// local manifest
					const FVxFilePakEntry& PakFile = It.Value->Entry;
					PakFileText += FString::Printf(TEXT("%s\t%llu\t%s\t-1\t/\n"), *PakFile.PakName, PakFile.PakSizeBytes, *PakFile.PakVersion);
				}
			}
		}

		// 파일 쓰기.
		FString ManifestPath = CacheFolder / MANIFEST_LOCAL;
		if (WriteStringAsUtf8TextFile(PakFileText, ManifestPath))
		{
			// 재 저장 됨.
			bNeedsManifestSave = false;
		}
	}
}
*/

void FNxDownloader::SaveLocalManifestJson(bool bForce)
{
	if (bForce || bNeedsManifestSave)
	{
		FString jsonString;
		TSharedRef< TJsonWriter<TCHAR> > writer = TJsonWriterFactory<>::Create(&jsonString);

		writer->WriteObjectStart();

		const UNxPatchSettings* settings = GetDefault<UNxPatchSettings>();

		TMap<FString, FString>  cachedManifestProps;
		TArray<FVxPakFileEntry> cachedManifest = ParseManifestJson(CacheFolder / settings->MANIFEST_CACHED_SERVER, &cachedManifestProps);
		const FString* fileVersion = cachedManifestProps.Find(PATCH_FILEVERSION);
		if (fileVersion != nullptr && fileVersion->IsEmpty() == false)
		{
			writer->WriteValue(PATCH_FILEVERSION, *fileVersion);
		}

		const FString* platform = cachedManifestProps.Find(PATCH_CONTENT_PLATFORM);
		if (platform != nullptr && platform->IsEmpty() == false)
		{
			writer->WriteValue(PATCH_CONTENT_PLATFORM, *platform);
		}

		const FString* contentVersion = cachedManifestProps.Find(PATCH_CONTENT_VERSION);
		if (contentVersion != nullptr && contentVersion->IsEmpty() == false)
		{
			writer->WriteValue(PATCH_CONTENT_VERSION, *contentVersion);
		}
		
		int32 numEntries = 0;
		for (const auto& It : MapPakFiles)
		{
			if (It.Value->bIsEmbedded == false)
			{
				if (It.Value->SizeOnDisk > 0 || It.Value->Download.IsValid())
				{
					++numEntries;
				}
			}
		}
		
		writer->WriteValue(PATCH_CONTENT_PAKCOUNT, numEntries);
		
		writer->WriteArrayStart(PATCH_CONTENT_PAKFILES);

		for (const auto& It : MapPakFiles)
		{
			if (!It.Value->bIsEmbedded)
			{
				if (It.Value->SizeOnDisk > 0 || It.Value->Download.IsValid())
				{
					// local manifest
					const FVxPakFileEntry& PakFile = It.Value->Entry;

					writer->WriteObjectStart();

					writer->WriteValue(PAKENTRY_NAME, PakFile.PakName);
					writer->WriteValue(PAKENTRY_BYTES, (int64)PakFile.PakBytes);
					writer->WriteValue(PAKENTRY_VERSION, PakFile.PakVersion);
					writer->WriteValue(PAKENTRY_TYPE, PakFile.PakType);
					writer->WriteValue(PAKENTRY_CHUNKID, PakFile.PakChunkId);
					writer->WriteValue(PAKENTRY_RELATIVERUL, PakFile.PakRelativeUrl);

					writer->WriteObjectEnd();
				}
			}
		}
		writer->WriteArrayEnd();

		writer->WriteValue(PATCH_CONTENT_DESC, TEXT("client patch"));
		writer->WriteObjectEnd();
		writer->Close();	// Essential last
				
		FString manifestPath = CacheFolder / settings->MANIFEST_LOCAL;
		if (WriteStringAsUtf8TextFile(jsonString, manifestPath))
		{
			// 재 저장 됨.
			bNeedsManifestSave = false;
		}
	}
}

void FNxDownloader::WaitForMounts()
{
	bool bWaiting = false;

	for (const auto& It : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = It.Value;

		if (chunk->MountTask != nullptr)
		{
			if (!bWaiting)
			{
				NxPrintDisplay(LogPatcher, TEXT("Waiting for chunk mounts to complete..."));
				bWaiting = true;
			}

			// 비동기 작업이 끝날 때까지 기다립니다.
			chunk->MountTask->EnsureCompletion(true);

			// 메인 스레드에서 작업 완료
			CompleteMountTask(*chunk);

			check(chunk->MountTask == nullptr);
		}
	}

	if (bWaiting)
	{
		NxPrintDisplay(LogPatcher, TEXT("...chunk mounts finished."));
	}
}

void FNxDownloader::CancelDownload(const TSharedRef<FNxPakFile>& pakFile, bool bResult)
{
	if (pakFile->Download.IsValid())
	{
		// 다운로드 자체를 취소
		pakFile->Download->Cancel(bResult);
		
		check(!pakFile->Download.IsValid());
	}
}

void FNxDownloader::UnmountPakFile(const TSharedRef<FNxPakFile>& pakFile)
{
	// 이미 마운트 해제된 경우, Skip
	if (pakFile->bIsMounted)
	{
		// unmount
		if (ensure(FCoreDelegates::OnUnmountPak.IsBound()))
		{
			FString fullPathOnDisk = (pakFile->bIsEmbedded ? EmbeddedFolder : CacheFolder) / pakFile->Entry.PakName;

			if (ensure(FCoreDelegates::OnUnmountPak.Execute(fullPathOnDisk)))
			{
				// 마운트 플래그 지우기
				pakFile->bIsMounted = false;
			}
			else
			{
				NxPrintError(LogPatcher, TEXT("Unable to unmount %s"), *fullPathOnDisk);
			}
		}
		else
		{
			NxPrintError(LogPatcher, TEXT("Unable to unmount %s because no OnUnmountPak is bound"), *pakFile->Entry.PakName);
		}
	}
}

FNxDownloader::EVxChunkStatus FNxDownloader::GetChunkStatus(int32 chunkId) const
{
	// 청크 유효성 확인
	const TSharedRef<FNxChunk>* chunkPtr = MapChunks.Find(chunkId);
	if (chunkPtr == nullptr)
	{
		return EVxChunkStatus::Unknown;
	}
	const FNxChunk& chunk = **chunkPtr;

	// pak 파일이 없으면 유효성 실패
	if (!ensure(chunk.PakFiles.Num() > 0))
	{
		return EVxChunkStatus::Unknown;
	}

	// 마운팅 되었는지 확인
	if (chunk.bIsMounted)
	{
		return EVxChunkStatus::Mounted;
	}

	// 다운로드 중인 팩수와, 완료된 팩 수 확인
	int32 numPaks = chunk.PakFiles.Num(), numCached = 0, numDownloading = 0;
	for (const TSharedRef<FNxPakFile>& pakFile : chunk.PakFiles)
	{
		if (pakFile->bIsCached)
		{
			++numCached;
		}
		else if (pakFile->Download.IsValid())
		{
			++numDownloading;
		}
	}

	if (numCached >= numPaks)
	{
		// 캐시됨
		return EVxChunkStatus::Cached;
	}
	else if (numCached + numDownloading >= numPaks)
	{
		// 일부 다운로드가 아직 진행 중.
		return EVxChunkStatus::Downloading;
	}
	else if (numCached + numDownloading > 0)
	{
		// 청크가 부분적으로 로컬에 캐시되어 탑재되지 않았으며 다운로드로 미진행중.
		//	- Manifest 업데이트에 의해 발생할수 있음.
		return EVxChunkStatus::Partial;
	}

	// 로컬 캐싱이 시작되지 않음
	return EVxChunkStatus::Remote;
}

void FNxDownloader::GetChunkIds(FString pakType, TArray<int32>& outChunkIds) const
{
	for (const auto& chunkItem : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = chunkItem.Value;

		for (int32 i = 0; i < chunk->PakFiles.Num(); ++i)
		{
			const TSharedRef<FNxPakFile>& pakFile = chunk->PakFiles[i];

			if( pakType.Equals(pakFile->Entry.PakType, ESearchCase::IgnoreCase) )
			{
				if(outChunkIds.Contains(chunk->ChunkId) == false)
				{
					outChunkIds.Add(chunk->ChunkId);
				}
				break;
			}
		}
	}
}

void FNxDownloader::GetAllChunkIds(TArray<int32>& outChunkIds) const
{
	MapChunks.GetKeys(outChunkIds);
}

// static
void FNxDownloader::DumpLoadedChunks()
{
#if !WITH_EDITOR
	TSharedRef<FNxDownloader> downloader = FNxDownloader::GetChecked();

	TArray<int32> chunkIdList;
	downloader->GetAllChunkIds(chunkIdList);

	NxPrintDisplay(LogPatcher, TEXT("Dumping loaded chunk status\n--------------------------"));

	for (int32 chunkId : chunkIdList)
	{
		EVxChunkStatus chunkStatus = downloader->GetChunkStatus(chunkId);

		NxPrintDisplay(LogPatcher, TEXT("Chunk #%d => %s"), chunkId, ChunkStatusToString(chunkStatus));
	}
#endif
}

//static 
const TCHAR* FNxDownloader::ChunkStatusToString(EVxChunkStatus status)
{
	switch (status)
	{
	case EVxChunkStatus::Mounted:		return TEXT("Mounted");
	case EVxChunkStatus::Cached:		return TEXT("Cached");
	case EVxChunkStatus::Downloading:	return TEXT("Downloading");
	case EVxChunkStatus::Partial:		return TEXT("Partial");
	case EVxChunkStatus::Remote:		return TEXT("Remote");
	case EVxChunkStatus::Unknown:		return TEXT("Unknown");

	default: return TEXT("Invalid");
	}
}

int FNxDownloader::FlushCache()
{
	IFileManager& fileManager = IFileManager::Get();

	// 모든 마운트가 완료될 때까지 기다립니다.
	WaitForMounts();

	NxPrintDisplay(LogPatcher, TEXT("Flushing chunk caches at %s"), *CacheFolder);

	int filesDeleted = 0, filesSkipped = 0;

	for (const auto& It : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = It.Value;

		// 마운트 대기 확인
		check(chunk->MountTask == nullptr); 

		// 백그라운드 다운로드 취소
		bool bDownloadPending = false;
		for (const TSharedRef<FNxPakFile>& pakFile : chunk->PakFiles)
		{
			if (pakFile->Download.IsValid() && !pakFile->Download->HasCompleted())
			{
				// 다운로드 중인 팩 건너뛰기
				bDownloadPending = true;
				break;
			}
		}

		// 앞단에서 다운로드 보류중인 청크 Skip
		if (bDownloadPending)
		{
			for (const TSharedRef<FNxPakFile>& pakFile : chunk->PakFiles)
			{
				if (pakFile->SizeOnDisk > 0)
				{
					// Skip 된 기록.
					NxPrintWarning(LogPatcher, TEXT("Could not flush %s (chunk %d) due to download in progress."), *pakFile->Entry.PakName, chunk->ChunkId);

					++filesSkipped;
				}
			}
		}
		else
		{
			// 팩 삭제
			for (const TSharedRef<FNxPakFile>& pakFile : chunk->PakFiles)
			{
				if (pakFile->SizeOnDisk > 0 && !pakFile->bIsEmbedded)
				{
					// 삭제 로그
					FString fullPathOnDisk = CacheFolder / pakFile->Entry.PakName;

					if (ensure(fileManager.Delete(*fullPathOnDisk)))
					{
						NxPrintLog(LogPatcher, TEXT("Deleted %s (chunk %d)."), *fullPathOnDisk, chunk->ChunkId);

						++filesDeleted;

						// 플래그 설정.
						pakFile->bIsCached = false;
						pakFile->SizeOnDisk = 0;

						bNeedsManifestSave = true;
					}
					else
					{
						// 에러 로그
						NxPrintError(LogPatcher, TEXT("Unable to delete %s"), *fullPathOnDisk);

						++filesSkipped;
					}
				}
			}
		}
	}

	// 매니페스트를 다시 저장
	SaveLocalManifestJson(false);

	NxPrintDisplay(LogPatcher, TEXT("Chunk cache flush complete. %d files deleted. %d files skipped."), filesDeleted, filesSkipped);

	return filesSkipped;
}

int FNxDownloader::ValidateCache()
{
	IFileManager& fileManager = IFileManager::Get();

	// 모든 마운트가 완료될 때까지 기다립니다.
	WaitForMounts();

	NxPrintDisplay(LogPatcher, TEXT("Starting inline chunk validation."));
	
	int validFiles = 0, invalidFiles = 0, skippedFiles = 0;

	for (const auto& It : MapPakFiles)
	{
		const TSharedRef<FNxPakFile>& pakFile = It.Value;

		if (pakFile->bIsCached && !pakFile->bIsEmbedded)
		{
			// 특정 해시 버전을 확인
			bool bFileIsValid = false;
			if (pakFile->Entry.PakVersion.StartsWith(TEXT("SHA1:")))
			{
				// sha1 해시 확인
				bFileIsValid = CheckFileSha1Hash(CacheFolder / pakFile->Entry.PakName, pakFile->Entry.PakVersion);
			}
			else
			{
				// 해시 버전 확인 불가.
				NxPrintWarning(LogPatcher, TEXT("Unable to validate %s with version '%s'."), *pakFile->Entry.PakName, *pakFile->Entry.PakVersion);
				++skippedFiles;
				continue;
			}

			// 유효성 검사
			if (bFileIsValid)
			{
				// 유효함
				NxPrintLog(LogPatcher, TEXT("%s matches hash '%s'."), *pakFile->Entry.PakName, *pakFile->Entry.PakVersion);
				++validFiles;
			}
			else
			{
				// 유효하지 않음
				NxPrintWarning(LogPatcher, TEXT("%s does NOT match hash '%s'."), *pakFile->Entry.PakName, *pakFile->Entry.PakVersion);
				++invalidFiles;

				// 잘못된 파일 삭제
				FString fullPathOnDisk = CacheFolder / pakFile->Entry.PakName;
				if (ensure(fileManager.Delete(*fullPathOnDisk)))
				{
					NxPrintLog(LogPatcher, TEXT("Deleted invalid pak %s (chunk %d)."), *fullPathOnDisk, pakFile->Entry.PakChunkId);
					pakFile->bIsCached = false;
					pakFile->SizeOnDisk = 0;
					bNeedsManifestSave = true;
				}
			}
		}
	}

	// 매니페스트를 다시 저장
	SaveLocalManifestJson(false);
	
	NxPrintDisplay(LogPatcher, TEXT("Chunk validation complete. %d valid, %d invalid, %d skipped"), validFiles, invalidFiles, skippedFiles);

	return invalidFiles;
}

void FNxDownloader::BeginLoadingMode(const FNxCallback& callback)
{
	// 유효한 콜백 없이는 로드 모드를 시작 불가
	check(callback); 

	// 이미 로딩 모드인지 확인
	if (PostLoadCallbacks.Num() > 0)
	{
		NxPrintLog(LogPatcher, TEXT("JoinLoadingMode"));

		// 기존 로드 모드가 완료될 때까지 대기
		PostLoadCallbacks.Add(callback);
		return;
	}

	// 로딩 모드 시작
	NxPrintLog(LogPatcher, TEXT("BeginLoadingMode"));

#if PLATFORM_ANDROID || PLATFORM_IOS
	FPlatformApplicationMisc::ControlScreensaver(FPlatformApplicationMisc::Disable);
#endif

	// 통계 재설정
	LoadingModeStats.ErrorTexts.Empty();
	LoadingModeStats.BytesDownloaded = 0;
	LoadingModeStats.FilesDownloaded = 0;
	LoadingModeStats.ChunksMounted	 = 0;
	LoadingModeStats.LoadingStartTime = FDateTime::UtcNow();

	// 아직 대기열에 아무것도 없는 경우 바인딩 콜백 전에 다시 계산
	ComputeLoadingStats(); 

	// 콜백 설정
	PostLoadCallbacks.Add(callback);

	LoadingCompleteLatch = 0;

	// 다음 프레임에서 다시 계산(그 때까지 큐에 아무것도 없으면 콜백을 실행합니다.
	TWeakPtr<FNxDownloader> WeakThisPtr = AsShared();

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakThisPtr](float dts) 
		{
			TSharedPtr<FNxDownloader> downloaderPtr = WeakThisPtr.Pin();
			if (!downloaderPtr.IsValid() || downloaderPtr->PostLoadCallbacks.Num() <= 0)
			{
				return false; // stop ticking
			}

			return downloaderPtr->UpdateLoadingMode();
		}));
}

bool FNxDownloader::UpdateLoadingMode()
{
	// 로딩 통계 재계산
	ComputeLoadingStats();

	// 로딩 모드의 끝을 확인
	if (LoadingModeStats.FilesDownloaded >= LoadingModeStats.TotalFilesToDownload && LoadingModeStats.ChunksMounted >= LoadingModeStats.TotalChunksToMount)
	{
		// 콜백을 실행하기 전에 로드가 최소 2프레임 동안 완료되었는지 확인합니다.
		// 이것은 로딩 화면에 무시할 수 있는 양의 시간을 추가하지만 종속 로드가 대기열에 들어갈 기회를 제공합니다.
		static const int32 NUM_CONSECUTIVE_IDLE_FRAMES_FOR_LOADING_COMPLETION = 5;

		if (++LoadingCompleteLatch >= NUM_CONSECUTIVE_IDLE_FRAMES_FOR_LOADING_COMPLETION)
		{
			// 로드 모드 종료
			NxPrintLog(LogPatcher, TEXT("EndLoadingMode (%d files downloaded, %d chunks mounted)"), LoadingModeStats.FilesDownloaded, LoadingModeStats.ChunksMounted);

#if PLATFORM_ANDROID || PLATFORM_IOS
			FPlatformApplicationMisc::ControlScreensaver(FPlatformApplicationMisc::Enable);
#endif

			// 모든 로딩 모드 완료 콜백 실행
			TArray<FNxCallback> callbacks = MoveTemp(PostLoadCallbacks);
			if (callbacks.Num() > 0)
			{
				PostLoadCallbacks.Empty(); 

				for (const auto& callback : callbacks)
				{
					// 수집된 ErrorMessage가 없으면 true
					callback(LoadingModeStats.ErrorTexts.IsEmpty());
				}
			}
			return false;
		}
	}
	else
	{
		// 잠금 재설정
		LoadingCompleteLatch = 0;
	}

	return true;
}

void FNxDownloader::ComputeLoadingStats()
{
	LoadingModeStats.TotalBytesToDownload = LoadingModeStats.BytesDownloaded;
	LoadingModeStats.TotalFilesToDownload = LoadingModeStats.FilesDownloaded;
	LoadingModeStats.TotalChunksToMount   = LoadingModeStats.ChunksMounted;

	// 모든 청크에 대한 루프
	for (const auto& It : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = It.Value;

		// 마운트 중인 경우 마운트할 파일을 추가하십시오.
		if (chunk->MountTask != nullptr)
		{
			++LoadingModeStats.TotalChunksToMount;
		}
	}

	// 다운로드 확인
	for (const TSharedRef<FNxPakFile>& pakFile : ArrDownloadRequests)
	{
		++LoadingModeStats.TotalFilesToDownload;

		if (pakFile->Download.IsValid())
		{
			LoadingModeStats.TotalBytesToDownload += pakFile->Entry.PakBytes - pakFile->Download->GetProgress();
		}
		else
		{
			LoadingModeStats.TotalBytesToDownload += pakFile->Entry.PakBytes;
		}
	}
}

void FNxDownloader::ExecuteNextTick(const FNxCallback& Callback, bool bSuccess)
{
	if (Callback)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Callback, bSuccess](float dts) {
			Callback(bSuccess);
			return false;
		}));
	}
}

void FNxDownloader::TryLoadBuildManifest(int tryNumber)
{
	// 로컬 빌드 매니페스트 로드
	const UNxPatchSettings* settings = GetDefault<UNxPatchSettings>();
	TMap<FString, FString>  cachedManifestProps;
	TArray<FVxPakFileEntry> cachedManifest = ParseManifestJson(CacheFolder / settings->MANIFEST_CACHED_SERVER, &cachedManifestProps);

	// CONTENT_VERSION 이 일치하는지 확인
	if (cachedManifestProps.FindOrAdd(PATCH_CONTENT_VERSION) != CurrentContentVersion)
	{
		// 구성된 CDN 이 없으면, 완료 시킴.
		if (BuildDataUrls.Num() <= 0)
		{
			NxPrintError(LogPatcher, TEXT("Unable to download build manifest. No CDN urls configured."));
			LoadingModeStats.ErrorTexts.Add( TEXT("Unable to download build manifest. (NoCDN)") );

			// 콜백 실행 및 지우기
			FNxCallback callback = MoveTemp(UpdateBuildCallback);
			ExecuteNextTick(callback, false);
			return;
		}
		// 일정 시도 횟수를 넘어갈 경우 Download 를 종료한다.
		if (tryNumber >= settings->PatchTryCount)
		{
			NxPrintError(LogPatcher, TEXT("Number of download attempts exceeded"));
			LoadingModeStats.ErrorTexts.Add(TEXT("Number of download attempts exceeded. (NoCDN)"));

			// 콜백 실행 및 지우기
			FNxCallback callback = MoveTemp(UpdateBuildCallback);
			ExecuteNextTick(callback, false);
			return;
		}

		// 첫 번째 다운로드 시도
		if (tryNumber <= 0)
		{
			// 다운로드
			TryDownloadBuildManifest(tryNumber);
			return;
		}

		

		// 다운로드를 다시 시작하기 전에 지연 계산 (최대 60초까지 지연)
		float secondsToDelay = tryNumber * 5.0f;
		if (secondsToDelay > 60)
		{
			secondsToDelay = 60;
		}

		// 지연으로 재시도 설정 
		NxPrintLog(LogPatcher, TEXT("Will re-attempt manifest download in %f seconds"), secondsToDelay);

		TWeakPtr<FNxDownloader> WeakThisPtr = AsShared();

		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThisPtr, tryNumber](float Unused)
			{
				TSharedPtr<FNxDownloader> downloaderPtr = WeakThisPtr.Pin();
				if (downloaderPtr.IsValid())
				{
					downloaderPtr->TryDownloadBuildManifest(tryNumber);
				}
				return false;
			}), secondsToDelay);

		return;
	}

	// Manifest 와 캐시되어 있는 파일간 관계를 구성한다.
	LoadManifest(cachedManifest);

	// 콜백 실행 및 지우기
	FNxCallback callback = MoveTemp(UpdateBuildCallback);
	ExecuteNextTick(callback, true);
}

// Manifest 파일 다운로드 진행
void FNxDownloader::TryDownloadBuildManifest(int tryNumber)
{
	check(BuildDataUrls.Num() > 0);

	const UNxPatchSettings* settings = GetDefault<UNxPatchSettings>();

	// CDN 에서 매니페스트를 다운로드한 다음 로드
	FString manifestFileName = FString::Printf(TEXT("%s-%s.json"), *settings->MANIFEST_SERVER, *CurrentContentVersion);
	
	FString url = BuildDataUrls[tryNumber % BuildDataUrls.Num()] / manifestFileName;

	NxPrintLog(LogPatcher, TEXT("Downloading build manifest (attempt #%d) from %s"), tryNumber+1, *url);

	// 루트 CDN 에서 매니페스트 다운로드
	FHttpModule& httpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	check(!ManifestRequest.IsValid());

	ManifestRequest = httpModule.Get().CreateRequest();
	ManifestRequest->SetURL(url);
	ManifestRequest->SetVerb(TEXT("GET"));
	TWeakPtr<FNxDownloader> WeakThisPtr = AsShared();
	FString cachedManifestFullPath = CacheFolder / settings->MANIFEST_CACHED_SERVER;

	// 요청 완료 처리.
	ManifestRequest->OnProcessRequestComplete().BindLambda([WeakThisPtr, tryNumber, cachedManifestFullPath] (FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess) 
		{
			// 성공하면 저장
			FString LastError = TEXT("");

			if (bSuccess && HttpResponse.IsValid())
			{
				const int32 httpStatus = HttpResponse->GetResponseCode();
				if (EHttpResponseCodes::IsOk(httpStatus))
				{
					// 매니페스트를 파일에 저장
					if (!WriteStringAsUtf8TextFile(HttpResponse->GetContentAsString(), cachedManifestFullPath))
					{
						NxPrintError(LogPatcher, TEXT("Failed to write manifest to '%s'"), *cachedManifestFullPath);
						//LastError = FText::Format(LOCTEXT("FailedToWriteManifest", "[Try {0}] Failed to write manifest."), FText::AsNumber(tryNumber));
						LastError = FString::Printf( TEXT("[Try %d] Failed to write manifest."), tryNumber);
					}
				}
				else
				{
					NxPrintError(LogPatcher, TEXT("HTTP %d while downloading manifest from '%s'"), httpStatus, *HttpRequest->GetURL());
					//LastError = FText::Format(LOCTEXT("ManifestHttpError_FailureCode", "[Try {0}] Manifest download failed (HTTP {1})"), FText::AsNumber(tryNumber), FText::AsNumber(httpStatus));
					LastError = FString::Printf( TEXT("[Try %d] Manifest download failed (HTTP %d)"), tryNumber, httpStatus);
				}
			}
			else
			{
				NxPrintError(LogPatcher, TEXT("HTTP connection issue while downloading manifest '%s'"), *HttpRequest->GetURL());
				//LastError = FText::Format(LOCTEXT("ManifestHttpError_Generic", "[Try {0}] Connection issues downloading manifest. Check your network connection..."), FText::AsNumber(tryNumber));
				LastError = FString::Printf(TEXT("[Try %d] Connection issues downloading manifest. Check your network connection..."), tryNumber);
			}

			// 로드 진행
			TSharedPtr<FNxDownloader> downloaderPtr = WeakThisPtr.Pin();
			if (!downloaderPtr.IsValid())
			{
				NxPrintWarning(LogPatcher, TEXT("Downloader was destroyed while downloading manifest '%s'"), *HttpRequest->GetURL());
				return;
			}

			downloaderPtr->ManifestRequest.Reset();

			if(LastError.IsEmpty() == false)
			{
				downloaderPtr->LoadingModeStats.ErrorTexts.Add(LastError);
			}

			downloaderPtr->TryLoadBuildManifest(tryNumber + 1);
		});

	ManifestRequest->ProcessRequest();
}

// 보류 중인 마운트가 완료되기를 기다리는 블록
//	- step1. 가비지를 수집하여 청크에 대한 참조를 정리합니다.
//	- step2. 새 청크에 대한 항목을 만듭니다.
//	- step3. 변경되는 모든 청크에 대해 다운로드를 취소하고 유효하지 않은 pak을 마운트 해제합니다.
//	- step4. 변경된 청크가 마운트된 경우 새 목록에서 마운트되지 않은 모든 팩을 인라인으로 마운트합니다 (순서대로).
//	- step5. 더 이상 존재하지 않는 청크를 언로드합니다 (다운로드를 취소하고 모든 팩을 마운트 해제).
void FNxDownloader::LoadManifest(const TArray<FVxPakFileEntry>& manifestPakFiles)
{
	NxPrintDisplay(LogPatcher, TEXT("Beginning manifest load."));

	// 모든 마운트가 완료될 때까지 기다립니다.
	WaitForMounts();

	// 가비지 수집 트리거
	CollectGarbage(RF_NoFlags);

	// 청크 ID별로 매니페스트 팩 그룹화 (순서 유지)
	TMap<int32, TArray<FVxPakFileEntry>> manifest;

	for (const FVxPakFileEntry& fileEntry : manifestPakFiles)
	{
		check(fileEntry.PakChunkId >= 0);

		manifest.FindOrAdd(fileEntry.PakChunkId).Add(fileEntry);
	}

	// 오래된 청크 맵 복사
	TMap<int32, TSharedRef<FNxChunk>>	  oldChunks   = MoveTemp(MapChunks);
	TMap<FString, TSharedRef<FNxPakFile>> oldPakFiles = MoveTemp(MapPakFiles);

	// 새로운 청크를 반복
	int32 numChunks = 0, numPaks = 0;
	for (const auto& It : manifest)
	{
		int32 chunkId = It.Key;
		
		// 새 청크 및 이전 pak 파일 추적
		TSharedPtr<FNxChunk> chunk;
		TArray<TSharedRef<FNxPakFile>> prevPakList;

		// 청크 생성 또는 재사용
		TSharedRef<FNxChunk>* oldChunk = oldChunks.Find(chunkId);
		if (oldChunk != nullptr)
		{
			// 이전 청크 위로 이동
			chunk = *oldChunk;
			check(chunk->ChunkId == chunkId);

			// don't clean it up later
			oldChunks.Remove(chunkId);

			// move out OldPakFiles
			prevPakList = MoveTemp(chunk->PakFiles);
		}
		else
		{
			// 새로운 청크팩 생성.
			chunk = MakeShared<FNxChunk>();
			chunk->ChunkId = chunkId;
		}

		// 새 맵에 청크 추가
		MapChunks.Add(chunk->ChunkId, chunk.ToSharedRef());

		// 새 pak 파일 찾기 또는 만들기
		check(chunk->PakFiles.Num() == 0);

		for (const FVxPakFileEntry& fileEntry : It.Value)
		{
			// 이 파일에 대한 기존 파일이 있는지 확인
			const TSharedRef<FNxPakFile>* existingFilePtr = oldPakFiles.Find(fileEntry.PakName);
			if (existingFilePtr != nullptr)
			{
				const TSharedRef<FNxPakFile>& existingFile = *existingFilePtr;
				if (existingFile->Entry.PakVersion == fileEntry.PakVersion)
				{
					// 버전이 일치하면 크기도 일치해야 한다.
					if(existingFile->Entry.PakBytes != fileEntry.PakBytes)
					{
						//check(existingFile->Entry.PakBytes == fileEntry.PakBytes);
						LoadingModeStats.ErrorTexts.Add(FString::Printf(TEXT("%s Pak Size MisMatch, [%s:%d], [%s:%d]"),
							*fileEntry.PakName,
							*existingFile->Entry.PakVersion, existingFile->Entry.PakBytes,
							*fileEntry.PakVersion, fileEntry.PakBytes ));
					}

					// 업데이트 및 목록에 추가(캐시에서 로드한 경우 ChunkId 및 RelativeUrl을 채울 수 있음)
					existingFile->Entry = fileEntry;
					chunk->PakFiles.Add(existingFile);
					MapPakFiles.Add(existingFile->Entry.PakName, existingFile);

					// 이전 pak 파일 목록에서 제거
					oldPakFiles.Remove(fileEntry.PakName);
					continue;
				}
			}

			// 새 항목 만들기
			TSharedRef<FNxPakFile> newFile = MakeShared<FNxPakFile>();
			newFile->Entry = fileEntry;
			chunk->PakFiles.Add(newFile);
			MapPakFiles.Add(newFile->Entry.PakName, newFile);

			// 자체 포함된 pak 파일과 일치하는지 확인
			const FVxPakFileEntry* cachedEntry = MapEmbeddedPaks.Find(fileEntry.PakName);
			if (cachedEntry != nullptr && cachedEntry->PakVersion == fileEntry.PakVersion)
			{
				newFile->bIsEmbedded = true;
				newFile->bIsCached = true;
				newFile->SizeOnDisk = cachedEntry->PakBytes;
			}
		}

		// 청크 및 pak 파일 수 기록
		UE_LOG(LogPatcher, Verbose, TEXT("Found chunk %d (%d pak files)."), chunkId, chunk->PakFiles.Num());
		++numChunks;
		numPaks += chunk->PakFiles.Num();

		// 청크가 이미 마운트된 경우 잘못된 데이터를 마운트 해제하려고 합니다.
		check(chunk->MountTask == nullptr); 
		
		// 이미 마운트가 완료되기를 기다렸습니다.
		if (chunk->bIsMounted)
		{
			// 모든 기존 pak 파일이 새 매니페스트와 일치하는지 확인.
			//	- 일치하면 마운팅은 Skip 가능.
			int longestCommonPrefix = 0;
			for (int i=0;i < prevPakList.Num() && i < chunk->PakFiles.Num();++i,++longestCommonPrefix)
			{
				if (chunk->PakFiles[i]->Entry.PakVersion != prevPakList[i]->Entry.PakVersion)
				{
					break;
				}
			}

			// 모두 일치하지 않으면 다시 마운트해야 합니다.
			if (longestCommonPrefix != prevPakList.Num() || longestCommonPrefix != chunk->PakFiles.Num())
			{
				// 이 청크는 더 이상 완전히 마운트되지 않습니다.
				chunk->bIsMounted = false;

				// 일치하지 않는 오래된 팩을 마운트 해제합니다 (역순).
				for (int i= prevPakList.Num()-1;i>=0;--i)
				{
					UnmountPakFile(prevPakList[i]);
				}

				// 일치하지 않는 새 팩을 마운트 해제합니다 (위치가 변경되었을 수 있음 : 역순).
				// 마운트 해제된 새 pak 파일은 이 청크가 다시 요청되면 올바른 순서로 다시 마운트됩니다.
				for (int i=chunk->PakFiles.Num()-1;i>=0;--i)
				{
					UnmountPakFile(chunk->PakFiles[i]);
				}
			}
		}
	}

	// OldPakFiles 에 남아 있는 모든 파일은 취소, 마운트 해제 및 삭제되어야 합니다.
	IFileManager& fileManager = IFileManager::Get();
	for (const auto& It : oldPakFiles)
	{
		const TSharedRef<FNxPakFile>& pakFile = It.Value;
		NxPrintLog(LogPatcher, TEXT("Removing orphaned pak file %s (was chunk %d)."), *pakFile->Entry.PakName, pakFile->Entry.PakChunkId);

		// 더 이상 유효하지 않은 pak 파일 다운로드 취소
		if (pakFile->Download.IsValid())
		{
			// pak이 더 이상 필요하지 않으므로 이러한 취소를 성공한 것으로 처리합니다(아무것도 성공적으로 다운로드하지 않음).
			CancelDownload(pakFile, true);
		}

		// 청크가 완전히 사라진 경우 이 방법으로 마운트를 정리해야 할 수 있습니다(그렇지 않으면 위에서 처리되었을 것입니다)
		if (pakFile->bIsMounted)
		{
			UnmountPakFile(pakFile);
		}

		// 로컬에 캐시된 파일 삭제
		if (pakFile->SizeOnDisk > 0 && !pakFile->bIsEmbedded)
		{
			bNeedsManifestSave = true;
			FString fullPathOnDisk = CacheFolder / pakFile->Entry.PakName;
			if (!ensure(fileManager.Delete(*fullPathOnDisk)))
			{
				NxPrintError(LogPatcher, TEXT("Failed to delete orphaned pak %s."), *fullPathOnDisk);
			}
		}
	}

	// 매니페스트를 다시 저장
	SaveLocalManifestJson(false);

	// 완료
	check(manifestPakFiles.Num() == numPaks);

	NxPrintDisplay(LogPatcher, TEXT("Manifest load complete. %d chunks with %d pak files."), numChunks, numPaks);
}

void FNxDownloader::DownloadChunkInternal(const FNxChunk& chunk, const FNxCallback& callback, int32 priority)
{
	NxPrintLog(LogPatcher, TEXT("Chunk %d download requested."), chunk.ChunkId);

	// 다운로드가 필요한지 확인
	bool bNeedsDownload = false;
	for (const auto& pakFile : chunk.PakFiles)
	{
		if (!pakFile->bIsCached)
		{
			bNeedsDownload = true;
			break;
		}
	}
	if (!bNeedsDownload)
	{
		ExecuteNextTick(callback, true);
		return;
	}

	// CDN이 구성되어 있는지 확인
	if (BuildDataUrls.Num() <= 0)
	{
		NxPrintError(LogPatcher, TEXT("Unable to download Chunk %d (no CDN urls)."), chunk.ChunkId);
		ExecuteNextTick(callback, false);
		return;
	}

	// 아직 캐시되지 않은 pak 파일 모두 다운로드
	FNxMultiCallback* multiCallback = new FNxMultiCallback(callback);
	for (const auto& pakFile : chunk.PakFiles)
	{
		if (!pakFile->bIsCached)
		{
			DownloadPakFileInternal(pakFile, multiCallback->AddPending(), priority);
		}
	}
	check(multiCallback->GetNumPending() > 0);
} 

// 다운로드를 완료한 후, 마운트 한다.
void FNxDownloader::MountChunkInternal(FNxChunk& chunk, const FNxCallback& callback)
{
	check(!chunk.bIsMounted);

	// 이미 보류 중인 마운트가 있는지 확인
	if (chunk.MountTask != nullptr)
	{
		if (callback)
		{
			chunk.MountTask->GetTask().PostMountCallbacks.Add(callback);
		}
		return;
	}

	// 다운로드를 트리거해야 하는지 확인
	bool bAllPaksCached = true;
	for (const auto& pakFile : chunk.PakFiles)
	{
		if (!pakFile->bIsCached)
		{
			bAllPaksCached = false;
			break;
		}
	}

	if (bAllPaksCached)
	{
		// 모든 pak 파일이 캐시된 경우 지금 마운트
		NxPrintLog(LogPatcher, TEXT("Chunk %d mount requested (%d pak sequence)."), chunk.ChunkId, chunk.PakFiles.Num());

		// pak 파일을 마운트하기 위해 백그라운드 작업을 시작하십시오.
		check(chunk.MountTask == nullptr);
		chunk.MountTask = new FVxMountTask();

		// 작업을 구성
		FNxPakMountWork& mountWork = chunk.MountTask->GetTask();

		mountWork.ChunkId		= chunk.ChunkId;
		mountWork.CacheFolder	= CacheFolder;
		mountWork.EmbeddedFolder= EmbeddedFolder;

		for (const TSharedRef<FNxPakFile>& PakFile : chunk.PakFiles)
		{
			if (PakFile->bIsMounted == false)
			{
				mountWork.PakFiles.Add(PakFile);
			}
		}

		if (callback)
		{
			mountWork.PostMountCallbacks.Add(callback);
		}

		// 백그라운드 마운트 작업 시작
		chunk.MountTask->StartBackgroundTask();

		// 마운트가 완료될 때까지 프레임별 알림 시작
		if (!MountTicker.IsValid())
		{
			MountTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FNxDownloader::UpdateMountTasks));
		}
	}
	else
	{
		// pak 파일 다운로드 대기
		TWeakPtr<FNxDownloader> WeakThisPtr = AsShared();
		int32 chunkId = chunk.ChunkId;
		DownloadChunkInternal(chunk, [WeakThisPtr, chunkId, callback](bool bDownloadSuccess)
			{
				// 다운로드에 실패하면 마운트할 수 없습니다.
				if (bDownloadSuccess)
				{
					TSharedPtr<FNxDownloader> downloaderPtr = WeakThisPtr.Pin();
					if (downloaderPtr.IsValid())
					{
						// 모든 청크가 다운로드되면 마운트를 다시 수행합니다(변경 사항을 선택하고 필요한 경우 다운로드를 계속함)
						downloaderPtr->MountChunk(chunkId, callback);
						return;
					}
				}

				// 문제가 발생하면, 콜백 호출.
				if (callback)
				{
					callback(false);
				}
			}, MAX_int32);
	}
}

void FNxDownloader::DownloadPakFileInternal(const TSharedRef<FNxPakFile>& pakFile, const FNxCallback& callback, int32 priority)
{
	check(BuildDataUrls.Num() > 0);

	// 업데이트된 경우 우선 순위를 높입니다.
	if (priority > pakFile->Priority)
	{
		// 다운로드가 이미 시작된 경우 실제로 아무 것도 변경되지 않습니다.
		pakFile->Priority = priority;
	}

	// 기존의 다운로드 후 콜백을 피기백하기만 하면 됩니다.
	if (callback)
	{
		pakFile->PostDownloadCallbacks.Add(callback);
	}

	// 다운로드가 이미 시작되었는지 확인
	if (pakFile->Download.IsValid())
	{
		return;
	}

	// 다운로드 세트에 추가
	ArrDownloadRequests.AddUnique(pakFile);
	ArrDownloadRequests.StableSort([](const TSharedRef<FNxPakFile>& A, const TSharedRef<FNxPakFile>& B) 
		{
		return A->Priority < B->Priority;
		});

	// 다운로드 시작
	IssueDownloads();
}

void FNxDownloader::IssueDownloads()
{
	for (int32 i = 0; i < ArrDownloadRequests.Num() && i < TargetDownloadsInFlight; ++i)
	{
		TSharedRef<FNxPakFile> downloadPakFile = ArrDownloadRequests[i];
		if (downloadPakFile->Download.IsValid())
		{
			// 이미 다운로드 중
			continue;
		}

		// 다운로드를 시작하고 있다는 기록
		NxPrintLog(LogPatcher, TEXT("Pak file %s download requested (%s)."),
			*downloadPakFile->Entry.PakName,
			*downloadPakFile->Entry.PakRelativeUrl
		);

		bNeedsManifestSave = true;

		// 새 다운로드 만들기 (플랫폼별)
		downloadPakFile->Download = MakeShared<FNxDownload>(AsShared(), downloadPakFile);
		downloadPakFile->Download->Start();
	}
}

void FNxDownloader::CompleteMountTask(FNxChunk& chunk)
{
	check(chunk.MountTask != nullptr);
	check(chunk.MountTask->IsDone());

	// 증분 청크 
	++LoadingModeStats.ChunksMounted;

	// 마운트 Task 해제
	FVxMountTask* mountTask = chunk.MountTask;
	chunk.MountTask = nullptr;

	const FNxPakMountWork& mountWork = mountTask->GetTask();

	// 실제로 성공한 팩에서 bIsMounted 업데이트
	for (const TSharedRef<FNxPakFile>& pakFile : mountWork.MountedPakFiles)
	{
		pakFile->bIsMounted = true;
	}

	// 청크에 bIsMounted 업데이트
	bool bAllPaksMounted = true;
	for (const TSharedRef<FNxPakFile>& pakFile : chunk.PakFiles)
	{
		if (pakFile->bIsMounted == false)
		{
			// FText::Format(LOCTEXT("FailedToMount", "Failed to mount {0}."), FText::FromString(pakFile->Entry.PakName));
			LoadingModeStats.ErrorTexts.Add(FString::Printf(TEXT("Failed to mount %s."), *pakFile->Entry.PakName));
			bAllPaksMounted = false;
			break;
		}
	}

	chunk.bIsMounted = bAllPaksMounted;

	if (chunk.bIsMounted)
	{
		NxPrintLog(LogPatcher, TEXT("Chunk %d mount succeeded."), chunk.ChunkId);
	}
	else
	{
		NxPrintError(LogPatcher, TEXT("Chunk %d mount failed."), chunk.ChunkId);
	}

	// 마운트 후 콜백 트리거
	for (const FNxCallback& callback : mountWork.PostMountCallbacks)
	{
		ExecuteNextTick(callback, bAllPaksMounted);
	}

	// 멀티캐스트 이벤트를 트리거합니다.
	OnChunkMounted.Broadcast(chunk.ChunkId, bAllPaksMounted);

	// 최종 Task 삭제
	delete mountTask;

	// 로딩 통계 재계산
	ComputeLoadingStats();
}

bool FNxDownloader::UpdateMountTasks(float dts)
{
	bool bMountsPending = false;

	for (const auto& It : MapChunks)
	{
		const TSharedRef<FNxChunk>& chunk = It.Value;
		if (chunk->MountTask != nullptr)
		{
			if (chunk->MountTask->IsDone())
			{
				// 완료
				CompleteMountTask(*chunk);
			}
			else
			{
				// 마운트가 아직 보류 중입니다.
				bMountsPending = true;
			}
		}
	}

	if (!bMountsPending)
	{
		MountTicker.Reset();
	}

	return bMountsPending; // keep ticking
}

void FNxDownloader::DownloadChunk(int32 chunkId, const FNxCallback& callback, int32 priority)
{
	// 청크 검색
	TSharedRef<FNxChunk>* chunkPtr = MapChunks.Find(chunkId);
	if (chunkPtr == nullptr || (*chunkPtr)->PakFiles.Num() <= 0)
	{
		// 존재하지 않는 청크 또는 pak 파일이 없는 청크는 모두 완료된 것으로 우선 처리함.
		//	- GetChunkStatus() 를 사용하여 성공적으로 마운트된 청크와 구별합니다.
		NxPrintWarning(LogPatcher, TEXT("Ignoring download request for chunk %d (no mapped pak files)."), chunkId);

		ExecuteNextTick(callback, true);

		return;
	}
	const FNxChunk& chunk = **chunkPtr;

	// 모든 Pak이 캐시 되면 완료.
	if (chunk.IsCached())
	{
		ExecuteNextTick(callback, true);
		return;
	}

	// 다운로드 대기열
	DownloadChunkInternal(chunk, callback, priority);

	// 필요한 경우 매니페스트 다시 저장
	SaveLocalManifestJson(false);

	ComputeLoadingStats();
}

void FNxDownloader::DownloadChunks(const TArray<int32>& chunkIds, const FNxCallback& callback, int32 priority)
{
	// 청크 참조로 변환
	TArray<TSharedRef<FNxChunk>> chunksToDownload;

	for (int32 chunkId : chunkIds)
	{
		TSharedRef<FNxChunk>* chunkPtr = MapChunks.Find(chunkId);
		if (chunkPtr != nullptr)
		{
			TSharedRef<FNxChunk>& chunkRef = *chunkPtr;
			if (chunkRef->PakFiles.Num() > 0)
			{
				if (chunkRef->IsCached() == false)
				{
					chunksToDownload.Add(chunkRef);
				}
				continue;
			}
		}

		NxPrintWarning(LogPatcher, TEXT("Ignoring download request for chunk %d (no mapped pak files)."), chunkId);
	}

	// 마운트할 청크가 있는지 확인
	if (chunksToDownload.Num() <= 0)
	{
		// Tick 알림.
		ExecuteNextTick(callback, true);
		return;
	}

#ifndef PVS_STUDIO // 빌드 머신 warning disable

	// Callback 이 없을경우, 관리없이 처리
	if (callback)
	{
		// 청크를 반복하고 개별 콜백을 발행
		FNxMultiCallback* multiCallback = new FNxMultiCallback(callback);
		for (const TSharedRef<FNxChunk>& chunk : chunksToDownload)
		{
			DownloadChunkInternal(*chunk, multiCallback->AddPending(), priority);
		}
		check(multiCallback->GetNumPending() > 0);
	}
	else
	{
		// 콜백을 관리할 필요가 없습니다.
		for (const TSharedRef<FNxChunk>& chunk : chunksToDownload)
		{
			DownloadChunkInternal(*chunk, FNxCallback(), priority);
		}
	}
#endif

	// 필요한 경우 매니페스트 다시 저장
	SaveLocalManifestJson(false);

	ComputeLoadingStats();
}

void FNxDownloader::MountChunk(int32 chunkId, const FNxCallback& callback)
{
	// 청크 검색
	TSharedRef<FNxChunk>* chunkPtr = MapChunks.Find(chunkId);

	if (chunkPtr == nullptr || (*chunkPtr)->PakFiles.Num() <= 0)
	{
		// 존재하지 않는 청크 또는 pak 파일이 없는 청크는 모두 완료된 것으로 우선 처리함.
		//	- GetChunkStatus() 를 사용하여 성공적으로 마운트된 청크와 구별합니다.
		NxPrintWarning(LogPatcher, TEXT("Ignoring mount request for chunk %d (no mapped pak files)."), chunkId);
		ExecuteNextTick(callback, false);
		return;
	}
	FNxChunk& chunk = **chunkPtr;

	// 이미 마운트되어 있는지 확인
	if (chunk.bIsMounted)
	{
		// Tick 알림.
		ExecuteNextTick(callback, true);
		return;
	}

	// 청크를 마운트
	MountChunkInternal(chunk, callback);

	// 필요한 경우 매니페스트 다시 저장
	SaveLocalManifestJson(false);

	ComputeLoadingStats();
}

void FNxDownloader::MountChunks(const TArray<int32>& chunkIds, const FNxCallback& callback)
{
	// 청크 참조로 변환
	TArray<TSharedRef<FNxChunk>> chunksToMount;

	for (int32 chunkId : chunkIds)
	{
		TSharedRef<FNxChunk>* chunkPtr = MapChunks.Find(chunkId);
		if (chunkPtr != nullptr)
		{
			TSharedRef<FNxChunk>& chunkRef = *chunkPtr;
			if (chunkRef->PakFiles.Num() > 0)
			{
				if (!chunkRef->bIsMounted)
				{
					chunksToMount.Add(chunkRef);
				}
				continue;
			}
		}
		NxPrintWarning(LogPatcher, TEXT("Ignoring mount request for chunk %d (no mapped pak files)."), chunkId);
	}

	// 마운트할 청크가 있는지 확인
	if (chunksToMount.Num() <= 0)
	{
		// Tick 알림.
		ExecuteNextTick(callback, true);
		return;
	}
		
#ifndef PVS_STUDIO // 빌드 머신 warning disable

	// Callback 이 없을경우, 관리없이 처리
	if (callback)
	{
		// 청크를 반복하고 개별 콜백을 발행.
		FNxMultiCallback* multiCallback = new FNxMultiCallback(callback);
		for (const TSharedRef<FNxChunk>& chunk : chunksToMount)
		{
			MountChunkInternal(*chunk, multiCallback->AddPending());
		}
		check(multiCallback->GetNumPending() > 0);
	} 
	else
	{
		// 콜백을 관리할 필요가 없습니다.
		for (const TSharedRef<FNxChunk>& chunk : chunksToMount)
		{
			MountChunkInternal(*chunk, FNxCallback());
		}
	}
#endif

	// 필요한 경우 매니페스트 다시 저장
	SaveLocalManifestJson(false);

	ComputeLoadingStats();
}

#undef LOCTEXT_NAMESPACE
