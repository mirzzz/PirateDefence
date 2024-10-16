// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodePatcherModule.h"
#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformChunkInstall.h"
#include "NxDownloader.h"
#include "Interfaces/IPluginManager.h"
#include "NxBaseLog.h"


class FNxPatcherPlatformWrapper : public FGenericPlatformChunkInstall
{
public:
	virtual EChunkLocation::Type GetChunkLocation(uint32 ChunkID) override
	{
		// 청크 0은 특별하고 항상 앱과 함께 제공되므로(정의에 따라) 누락된 것으로 보고하지 마십시오.
		if (ChunkID == 0)
		{
			return EChunkLocation::BestLocation;
		}

		// 청크 다운로더가 초기화되지 않은 경우 누락된 모든 청크를 보고합니다.
		if (!NxCodePatcher.IsValid())
		{
			return EChunkLocation::DoesNotExist;
		}

		// 다운로더의 상태를 청크 설치 인터페이스 열거형에 매핑
		switch (NxCodePatcher->GetChunkStatus(ChunkID))
		{
		case FNxDownloader::EVxChunkStatus::Mounted:
			return EChunkLocation::BestLocation;

		case FNxDownloader::EVxChunkStatus::Remote:
		case FNxDownloader::EVxChunkStatus::Partial:
		case FNxDownloader::EVxChunkStatus::Downloading:
		case FNxDownloader::EVxChunkStatus::Cached:
			return EChunkLocation::NotAvailable;
		}
		return EChunkLocation::DoesNotExist;
	}

	virtual bool PrioritizeChunk(uint32 ChunkID, EChunkPriority::Type Priority) override
	{
		if (!NxCodePatcher.IsValid())
		{
			return false;
		}

		NxCodePatcher->MountChunk(ChunkID, FNxDownloader::FNxCallback());
		return true;
	}

	virtual FDelegateHandle AddChunkInstallDelegate(FPlatformChunkInstallDelegate Delegate) override
	{
		// create if necessary
		if (!NxCodePatcher.IsValid())
		{
			NxCodePatcher = MakeShareable(new FNxDownloader());
		}
		return NxCodePatcher->OnChunkMounted.Add(Delegate);
	}

	virtual void RemoveChunkInstallDelegate(FDelegateHandle Delegate) override
	{
		if (!NxCodePatcher.IsValid())
		{
			return;
		}
		NxCodePatcher->OnChunkMounted.Remove(Delegate);
	}

public: // trivial
	virtual EChunkInstallSpeed::Type GetInstallSpeed() override { return EChunkInstallSpeed::Fast; }
	virtual bool SetInstallSpeed(EChunkInstallSpeed::Type InstallSpeed) override { return false; }
	virtual bool DebugStartNextChunk() override { return false; }
	virtual bool GetProgressReportingTypeSupported(EChunkProgressReportingType::Type ReportType) override { return false; }
	virtual float GetChunkProgress(uint32 ChunkID, EChunkProgressReportingType::Type ReportType) override { return 0; }

public:

	FNxPatcherPlatformWrapper(TSharedPtr<FNxDownloader>& VxDownloaderRef) : NxCodePatcher(VxDownloaderRef)
	{
	}

	virtual ~FNxPatcherPlatformWrapper()
	{
	}

private:

	TSharedPtr<FNxDownloader>& NxCodePatcher;
};

/**
* Mcp Profile System Module
*/
class FNxCodePatcherModule : public IPlatformChunkInstallModule
{
public:
	FNxCodePatcherModule() : DownloaderWrapper(new FNxPatcherPlatformWrapper(Downloader))
	{
	}

	virtual IPlatformChunkInstall* GetPlatformChunkInstall() override
	{
		return DownloaderWrapper.Get();
	}

	// IModuleInterface interface
	virtual void StartupModule() override
	{
		NxPrintTxt(LogPatcher,  TEXT("[-------------------------------------------------------- NxCodePatcher Module Startup]"));

		NxPrintTodo(LogPatcher, TEXT("[# VisualStudio 솔루션 구성:DebugGame으로 실행시 문제가 발생하고 있음!"));
		NxPrintTodo(LogPatcher, TEXT("[-- platformFile.OpenWrite() 에서 파일을 생성 못 하고 nullptr 반환됨 - 2024.2"));
		//D:\unrealengine\Engine\Binaries\Win64\UnrealPak.exe D:\pakChunk0-Windows.pak -Extract D:\TempUnpack - 패키징 내용 확인
	} 

	virtual void ShutdownModule() override
	{
		if (Downloader.IsValid())
		{
			Downloader->Finalize();
		}
		NxPrintTxt(LogPatcher, TEXT("[------------------------------------------------------- NxCodePatcher Module Shutdown]"));
	}

	TSharedPtr<FNxDownloader>				 Downloader;
	TUniquePtr<FNxPatcherPlatformWrapper>	 DownloaderWrapper;
};

static const FName DownloaderModuleName = "NxCodePatcher";

//static 
TSharedPtr<FNxDownloader> FNxDownloader::Get()
{
	FNxCodePatcherModule* downloaderModule = FModuleManager::LoadModulePtr<FNxCodePatcherModule>(DownloaderModuleName);
	if (downloaderModule != nullptr)
	{
		// may still be null
		return downloaderModule->Downloader;
	}
	return TSharedPtr<FNxDownloader>();
}

//static 
TSharedRef<FNxDownloader> FNxDownloader::GetChecked()
{
	FNxCodePatcherModule& downloaderModule = FModuleManager::LoadModuleChecked<FNxCodePatcherModule>(DownloaderModuleName);
	return downloaderModule.Downloader.ToSharedRef();
}

//static 
TSharedRef<FNxDownloader> FNxDownloader::GetOrCreate()
{
	FNxCodePatcherModule& downloaderModule = FModuleManager::LoadModuleChecked<FNxCodePatcherModule>(DownloaderModuleName);
	if (downloaderModule.Downloader.IsValid() == false)
	{
		downloaderModule.Downloader = MakeShareable(new FNxDownloader());
	}
	return downloaderModule.Downloader.ToSharedRef();
}

//static 
void FNxDownloader::Shutdown()
{
	FNxCodePatcherModule* downloaderModule = FModuleManager::LoadModulePtr<FNxCodePatcherModule>(DownloaderModuleName);
	if (downloaderModule != nullptr)
	{
		// may still be null
		if (downloaderModule->Downloader.IsValid())
		{
			downloaderModule->Downloader->Finalize();
			downloaderModule->Downloader.Reset();
		}
	}
}

IMPLEMENT_MODULE(FNxCodePatcherModule, NxCodePatcher);

