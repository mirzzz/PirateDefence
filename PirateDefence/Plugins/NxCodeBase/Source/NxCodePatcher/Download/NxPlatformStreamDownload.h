// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 함수 포인터
typedef TFunction<void(int32 HttpStatus)>		FOnDownloadComplete;
typedef TFunction<void(int32 BytesReceived)>	FOnDownloadProgress;
typedef TFunction<void(void)>					FOnDownloadCancel;

// 파일 다운로드 함수
extern FOnDownloadCancel NxPlatformStreamDownload(const FString& Url, const FString& TargetFile, const FOnDownloadProgress& Progress, const FOnDownloadComplete& Callback);
 