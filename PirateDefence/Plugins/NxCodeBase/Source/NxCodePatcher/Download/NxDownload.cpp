// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxDownload.h"
#include "NxCodePatcherModule.h"
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxDownload"

static const int MAX_DOWNLOAD_ATTEMPTS = 3;

FNxDownload::FNxDownload(const TSharedRef<FNxDownloader>& DownloaderIn, const TSharedRef<FNxDownloader::FNxPakFile>& PakFileIn)
	: Downloader(DownloaderIn)
	, PakFile(PakFileIn)
	, TargetFile(Downloader->CacheFolder / PakFileIn->Entry.PakName)
{
	// couple of sanity checks for our flags
	check(!PakFile->bIsCached);
	check(!PakFile->bIsEmbedded);
	check(!PakFile->bIsMounted);
}

FNxDownload::~FNxDownload()
{
}

void FNxDownload::Start()
{
	check(!bHasCompleted);

	// 이 다운로드를 위한 충분한 공간이 있는지 확인
	if (!HasDeviceSpaceRequired())
	{
		TWeakPtr<FNxDownload> WeakThisPtr = AsShared();

		FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([WeakThisPtr](float Unused)
			{
				TSharedPtr<FNxDownload> downloadPtr = WeakThisPtr.Pin();
				if (downloadPtr.IsValid())
				{
					downloadPtr->OnCompleted(false, TEXT("Not enough space on device."));
				}
				return false;
			}
		), 1.0);

		return;
	}

	// CDN 에서 다운로드 시도
	StartDownload(0);
}

void FNxDownload::Cancel(bool bResult)
{
	check(!bHasCompleted);
	NxPrintWarning(LogPatcher, TEXT("Canceling download of '%s'. result=%s"), *PakFile->Entry.PakName, bResult ? TEXT("true") : TEXT("false"));

	// 플랫폼별 파일 다운로드 취소
	if (!bIsCancelled)
	{
		bIsCancelled = true;
		CancelCallback();
	}

	// 실패 완료 처리
	// FText::Format(LOCTEXT("DownloadCanceled", "Download of '%s' was canceled."), FText::FromString(PakFile->Entry.PakName))
	OnCompleted(bResult, FString::Printf(TEXT("Download of '%s' was canceled."), *PakFile->Entry.PakName));
}

void FNxDownload::UpdateFileSize()
{
	// 파일 크기 갱신
	IFileManager& fileManager = IFileManager::Get();
	int64 fileSizeOnDisk = fileManager.FileSize(*TargetFile);
	PakFile->SizeOnDisk = (fileSizeOnDisk > 0) ? (uint64)fileSizeOnDisk : 0;
}

bool FNxDownload::ValidateFile() const
{
	if (PakFile->SizeOnDisk != PakFile->Entry.PakBytes)
	{
		NxPrintError(LogPatcher, TEXT("Size mismatch. Expected %llu, got %llu"), PakFile->Entry.PakBytes, PakFile->SizeOnDisk);
		return false;
	}

	if (PakFile->Entry.PakVersion.StartsWith(TEXT("SHA1:")))
	{
		// sha1 해시 확인
		if (!FNxDownloader::CheckFileSha1Hash(TargetFile, PakFile->Entry.PakVersion))
		{
			NxPrintError(LogPatcher, TEXT("Checksum mismatch. Expected %s"), *PakFile->Entry.PakVersion);
			return false;
		}
	}

	return true;
}

// 저장소 공간 확인
bool FNxDownload::HasDeviceSpaceRequired() const
{
	uint64 totalDiskSpace = 0;
	uint64 totalDiskFreeSpace = 0;

	if (FPlatformMisc::GetDiskTotalAndFreeSpace(Downloader->CacheFolder, totalDiskSpace, totalDiskFreeSpace))
	{
		uint64 bytesNeeded = (PakFile->Entry.PakBytes > PakFile->SizeOnDisk) ? PakFile->Entry.PakBytes - PakFile->SizeOnDisk : PakFile->SizeOnDisk - PakFile->Entry.PakBytes;
		if (totalDiskFreeSpace < bytesNeeded)
		{
			// 공간이 충분하지 않습니다
			NxPrintWarning(LogPatcher, TEXT("Unable to download '%s'. Needed %llu bytes had %llu bytes free (of %llu bytes)"),
				*PakFile->Entry.PakName, PakFile->Entry.PakBytes, totalDiskFreeSpace, totalDiskSpace);
			return false;
		}
	}
	return true;
}

void FNxDownload::StartDownload(int tryNumber)
{
	// 한 번만 완료 처리
	check(!bHasCompleted);
	BeginTime = FDateTime::UtcNow();

	OnDownloadProgress(0);

	// 다음 URL을 다운로드
	check(Downloader->BuildDataUrls.Num() > 0);
	FString url = Downloader->BuildDataUrls[tryNumber % Downloader->BuildDataUrls.Num()] / PakFile->Entry.PakRelativeUrl;
	NxPrintLog(LogPatcher, TEXT("Downloading %s from %s"), *PakFile->Entry.PakName, *url);

	TWeakPtr<FNxDownload> WeakThisPtr = AsShared();

	CancelCallback = NxPlatformStreamDownload(url, TargetFile, 
		
		// FDownloadProgress
		[WeakThisPtr](int32 BytesReceived) 
		{
			TSharedPtr<FNxDownload> downloadPtr = WeakThisPtr.Pin();
			if (downloadPtr.IsValid() && !downloadPtr->bHasCompleted)
			{
				downloadPtr->OnDownloadProgress(BytesReceived);
			}
		}, 
		
		// FDownloadComplete
		[WeakThisPtr, tryNumber, url](int32 HttpStatus) 
		{
			TSharedPtr<FNxDownload> downloadPtr = WeakThisPtr.Pin();
			if (downloadPtr.IsValid() && !downloadPtr->bHasCompleted)
			{
				downloadPtr->OnDownloadComplete(url, tryNumber, HttpStatus);
			}
		});
}

void FNxDownload::OnDownloadProgress(int32 bytesReceived)
{
	Downloader->LoadingModeStats.BytesDownloaded -= LastBytesReceived;
	LastBytesReceived = bytesReceived;
	Downloader->LoadingModeStats.BytesDownloaded += LastBytesReceived;
}

void FNxDownload::OnDownloadComplete(const FString& url, int tryNumber, int32 httpStatus)
{
	// 한 번만 완료 처리
	check(!bHasCompleted);

	// 디스크의 파일 크기 업데이트
	UpdateFileSize();

	// 보고서 분석
	if (Downloader->OnDownloadAnalytics)
	{
		Downloader->OnDownloadAnalytics(PakFile->Entry.PakName, url, PakFile->SizeOnDisk, FDateTime::UtcNow() - BeginTime, httpStatus);
	}

	// 완료 처리
	if (EHttpResponseCodes::IsOk(httpStatus))
	{
		// 파일이 완전한지 확인
		if (ValidateFile() == true)
		{
			PakFile->bIsCached = true;
			OnCompleted(true, TEXT(""));
			return;
		}
		
		// 유효성 검사에 실패
		NxPrintError(LogPatcher, TEXT("%s from %s failed validation"), *TargetFile, *url);

		// 파일을 삭제하고 다시 다운로드를 진행. (Network 문제로 파일에 문제가 발생할수 있음)
		IPlatformFile::GetPlatformPhysical().DeleteFile(*TargetFile);
	}

	// 파일 다운로드를 위한 충분한 공간이 있는지 다시 확인
	if (!HasDeviceSpaceRequired())
	{
		OnCompleted(false, TEXT("Not enough space on device."));
		return;
	}

	// 다운로드 재시도를 3 회 이상한 경우, 강제 종료
	if (tryNumber > MAX_DOWNLOAD_ATTEMPTS)
	{
		OnCompleted(false, FString::Printf( TEXT("%s Ended with download instability issue."), *PakFile->Entry.PakName ));
		return;
	}

	// 다운로드를 다시 시작하기 전에 지연 계산 (5초씩 증가)
	float secondsToDelay = (tryNumber + 1) * 5.0f;		
	if (secondsToDelay > 60)
	{
		secondsToDelay = 60;
	}

	// 다운로드 재시도 지연 시간 로그.
	NxPrintLog(LogPatcher, TEXT("Will re-attempt to download %s in %f seconds"), *PakFile->Entry.PakName, secondsToDelay);

	TWeakPtr<FNxDownload> WeakThisPtr = AsShared();
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThisPtr, tryNumber](float Unused) 
		{
			TSharedPtr<FNxDownload> downloadPtr = WeakThisPtr.Pin();
			if (downloadPtr.IsValid() && !downloadPtr->bHasCompleted)
			{
				downloadPtr->StartDownload(tryNumber + 1);
			}
			return false;

		}), secondsToDelay);
}

void FNxDownload::OnCompleted(bool bSuccess, const FString& ErrorText)
{
	// 한 번 이상 완료하지 않도록 확인
	check(!bHasCompleted);
	bHasCompleted = true;

	// 다운로드된 파일 갯수 증가
	OnDownloadProgress(bSuccess ? PakFile->SizeOnDisk : 0);

	++Downloader->LoadingModeStats.FilesDownloaded;

	if (!bSuccess && !ErrorText.IsEmpty())
	{
		Downloader->LoadingModeStats.ErrorTexts.Add(ErrorText);
	}

	// 콜백 대기
	for (const auto& Callback : PakFile->PostDownloadCallbacks)
	{
		Downloader->ExecuteNextTick(Callback, bSuccess);
	}
	PakFile->PostDownloadCallbacks.Empty();

	// 다운로드 요청에서 제거
	if (ensure(Downloader->ArrDownloadRequests.RemoveSingle(PakFile) > 0))
	{
		Downloader->IssueDownloads();
	}

	// pak 파일에서 후크 해제(이것은 우리를 삭제할 수 있음)
	if (PakFile->Download.Get() == this)
	{
		PakFile->Download.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
