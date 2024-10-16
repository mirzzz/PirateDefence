// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NxDownloader.h"
#include "Misc/CoreMisc.h"
#include "HAL/FileManagerGeneric.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "PlatformHttp.h"
#include "NxPlatformStreamDownload.h"

class FNxDownload : public TSharedFromThis<FNxDownload>
{
public:

	FNxDownload(const TSharedRef<FNxDownloader>& DownloaderIn, const TSharedRef<FNxDownloader::FNxPakFile>& PakFileIn);
	virtual ~FNxDownload();

	inline bool HasCompleted() const { return bHasCompleted; }
	inline int32 GetProgress() const { return LastBytesReceived; }

	void Start();
	void Cancel(bool bResult);

public:

	const TSharedRef<FNxDownloader>				Downloader;
	const TSharedRef<FNxDownloader::FNxPakFile> PakFile;

	const FString								TargetFile;

protected:
	void UpdateFileSize();
	bool ValidateFile() const;
	bool HasDeviceSpaceRequired() const;
	void StartDownload(int TryNumber);
	void OnDownloadProgress(int32 BytesReceived);
	void OnDownloadComplete(const FString& Url, int TryNumber, int32 HttpStatus);
	void OnCompleted(bool bSuccess, const FString& ErrorText);

private:
	bool				bIsCancelled = false;
	FOnDownloadCancel	CancelCallback;
	bool				bHasCompleted = false;
	FDateTime			BeginTime;
	int32				LastBytesReceived = 0;
};
