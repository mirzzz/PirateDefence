// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxPlatformStreamDownload.h"
#include "NxCodePatcherModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "NxBaseLog.h"
 
//////////////////////////////////////////////////////////////////////////////////
#if 0 && PLATFORM_ANDROID

// Android
// https://developer.android.com/reference/android/app/DownloadManager.html
#error "TODO: android"
FDownloadCancel NxPlatformStreamDownload(const FString& Url, const FString& TargetFile, const FDownloadProgress& Progress, const FDownloadComplete& Callback)
{
	// TODO: write me
	Callback(0);
	// not cancelable
	return []() {};
}

//////////////////////////////////////////////////////////////////////////////////
#elif 0 && PLATFORM_IOS

// iOS
// https://developer.apple.com/library/content/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/BackgroundExecution/BackgroundExecution.html
#error "TODO: ios"
FOnDownloadCancel NxPlatformStreamDownload(const FString& Url, const FString& TargetFile, const FOnDownloadProgress& Progress, const FOnDownloadComplete& Callback)
{
	// TODO: write me
	Callback(0);
	// not cancelable
	return []() {};
}

//////////////////////////////////////////////////////////////////////////////////
#else

// 참고: 파일을 스트리밍하지 않고 전체를 메모리에 로드한 후, 저장합니다 (최적은 아님). 
//		중단된 다운로드를 재개하려고 시도하지만 (테스트에 사용하기 위해) 
//		부분 쓰기를 수행하지 않기 때문에 이러한 작업을 위해서는 추가 작업이 필요합니다.
FOnDownloadCancel NxPlatformStreamDownload(const FString& Url, const FString& TargetFile, const FOnDownloadProgress& Progress, const FOnDownloadComplete& Callback)
{
	// 현재 디스크에 있는 파일의 양 (있는 경우)
	IFileManager& FileManager = IFileManager::Get();
	int64 FileSizeOnDisk = FileManager.FileSize(*TargetFile);
	uint64 SizeOnDisk = (FileSizeOnDisk > 0) ? (uint64)FileSizeOnDisk : 0;

	// 누락된 부품에 대한 범위 요청을 수행합니다.
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	if (SizeOnDisk > 0)
	{
		// 특정 범위를 요청
		Request->SetHeader(TEXT("Range"), FString::Printf(TEXT("bytes=%llu-"), SizeOnDisk));
	}

	// 진행 대리자 바인딩
	if (Progress)
	{
		Request->OnRequestProgress64().BindLambda([Progress](FHttpRequestPtr HttpRequest, int32 BytesSent, int32 BytesReceived)
			{
				Progress(BytesReceived);
			});
	}
	
	// 완료 대리자 바인딩
	Request->OnProcessRequestComplete().BindLambda([Callback, TargetFile, SizeOnDisk](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess) 
		{
			// 응답 확인
			int32 HttpStatus = 0;
			if (HttpResponse.IsValid())
			{
				HttpStatus = HttpResponse->GetResponseCode();
				bool bHeadersOk = EHttpResponseCodes::IsOk(HttpStatus);
				const bool bIsPartialContent = (HttpStatus == 206);
				if (bIsPartialContent)
				{
					static const FString ContentRangeHeader = TEXT("Content-Range");

					// 부분적인 내용이 있는 경우, Content-Range 헤더가 예상한 것과 같은지 확인.
					FString ExpectedHeaderPrefix = FString::Printf(TEXT("bytes %llu-"), SizeOnDisk);
					FString HeaderValue = HttpResponse->GetHeader(ContentRangeHeader);
					if (!HeaderValue.StartsWith(ExpectedHeaderPrefix))
					{
						NxPrintError(LogPatcher, TEXT("Content-Range for %s was '%s' but expected '%s' prefix"), *HttpRequest->GetURL(), *HeaderValue, *ExpectedHeaderPrefix);
						bHeadersOk = false;
					}
				}

				// 헤더가 괜찮은지 확인
				if (bHeadersOk)
				{
					// 쓰기 위해 파일을 엽니다
					IFileHandle* ManifestFile = IPlatformFile::GetPlatformPhysical().OpenWrite(*TargetFile, SizeOnDisk > 0 && bIsPartialContent);
					if (ManifestFile != nullptr)
					{
						// 파일에 쓰기
						const TArray<uint8>& Content = HttpResponse->GetContent();
						bSuccess = ManifestFile->Write(&Content[0], Content.Num());
						// 파일 닫기
						delete ManifestFile;

						// 실패
						if (!bSuccess)
						{
							NxPrintError(LogPatcher, TEXT("Write error writing to %s"), *TargetFile);

							// 파일 삭제
							IPlatformFile::GetPlatformPhysical().DeleteFile(*TargetFile);
						}
					}
					else
					{
						NxPrintError(LogPatcher, TEXT("Unable to save file to %s"), *TargetFile);

						// 파일 삭제
						if (SizeOnDisk > 0)
						{
							IPlatformFile::GetPlatformPhysical().DeleteFile(*TargetFile);
						}
					}
				}
				else
				{
					NxPrintError(LogPatcher, TEXT("HTTP %d returned from '%s'"), HttpStatus, *HttpRequest->GetURL());

					// 서버가 non-ok (서버 오류가 아님)로 응답하면 다음 번을 위해 파일을 삭제.
					if (HttpStatus < 500 && SizeOnDisk > 0)
					{
						IPlatformFile::GetPlatformPhysical().DeleteFile(*TargetFile);
					}
				}
			}
			else
			{
				NxPrintError(LogPatcher, TEXT("HTTP connection issue downloading '%s'"), *HttpRequest->GetURL());
			}

			// 콜백 호출
			if (Callback)
			{
				Callback(HttpStatus);
			}
		});
	
	Request->ProcessRequest();

	return [Request]() {
		Request->CancelRequest();
	};
}
#endif
