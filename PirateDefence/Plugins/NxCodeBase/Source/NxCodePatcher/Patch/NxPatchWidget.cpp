// Fill out your copyright notice in the Description page of Project Settings.

#include "NxPatchWidget.h"

// UMG
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/EditableText.h"
#include "Components/Overlay.h"

#include "Engine/GameInstance.h"
#include "Misc/DefaultValueHelper.h"

#include "NxPatchSubsystem.h"
#include "NxPatchSettings.h"



bool UNxPatchWidget::Initialize()
{
	return Super::Initialize();
}

void UNxPatchWidget::RemoveFromParent()
{
	Super::RemoveFromParent();
}

void UNxPatchWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Button Binding
	BT_StartMap->OnClicked.AddDynamic(this, &ThisClass::ClickStartMap);
	BT_PatchInitialize->OnClicked.AddDynamic(this, &ThisClass::ClickPatchInitialize);
	BT_PatchShutdown->OnClicked.AddDynamic(this, &ThisClass::ClickPatchShutdown);
	BT_Chunking->OnClicked.AddDynamic(this, &ThisClass::ClickChunking);
}

void UNxPatchWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNxPatchWidget::NativeDestruct()
{
	BT_StartMap->OnClicked.RemoveDynamic(this, &ThisClass::ClickStartMap);
	BT_PatchInitialize->OnClicked.RemoveDynamic(this, &ThisClass::ClickPatchInitialize);
	BT_PatchShutdown->OnClicked.RemoveDynamic(this, &ThisClass::ClickPatchShutdown);
	BT_Chunking->OnClicked.RemoveDynamic(this, &ThisClass::ClickChunking);

	Super::NativeDestruct();
}

void UNxPatchWidget::ShowLayout(bool show /*= true*/)
{
	// UCanvasPanel
	if (CP_Layout)
	{
		if (show)
		{
			CP_Layout->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CP_Layout->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UNxPatchWidget::ShowMenu(bool show /*= true*/)
{
	// UOverlay
	if (OL_Menu)
	{
		if (show)
		{
			OL_Menu->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			OL_Menu->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UNxPatchWidget::SetMessage(FText msg)
{
	ShowLayout(true);

	if (TB_NotifyText)
	{
		TB_NotifyText->SetText(msg);
	}
}

void UNxPatchWidget::SetErrorMessage(FText msg)
{
	SetMessage(msg);
	SetPatchReady(false);
}

void UNxPatchWidget::ResetProgressBar(float percent /*= 0.0f*/)
{
	FString blank = TEXT("");
	SetPatchProgressBar(percent, blank);
	SetMountProgressBar(percent, blank);
}

void UNxPatchWidget::SetPatchProgressBar(float percent, FString text)
{
	PB_PatchDownload->SetPercent(percent);
	
	FText patchText = FText::FromString(text);
	TB_PatchText->SetText(patchText);
}

void UNxPatchWidget::SetMountProgressBar(float percent, FString text)
{
	PB_Mount->SetPercent(percent);

	FText mountText = FText::FromString(text);
	TB_MountText->SetText(mountText);
}

void UNxPatchWidget::ClickStartMap()
{
	if (GetPatchSubSystem() )
	{
		GetPatchSubSystem()->GotoNextLevel();
	}
}

UNxPatchSubsystem* UNxPatchWidget::GetPatchSubSystem()
{
	if (UGameInstance* gameInstance = GetGameInstance())
	{
		UNxPatchSubsystem* patchSubsystem = gameInstance->GetSubsystem<UNxPatchSubsystem>();
		return patchSubsystem;
	}
	return nullptr;
}

void UNxPatchWidget::ClickPatchInitialize()
{
	if (IsPatchReady == false)
	{
		IsPatchReady = true;

		if (GetPatchSubSystem())
		{
			// 패칭 시작
			GetPatchSubSystem()->InitPatching(GetDefault<UNxPatchSettings>()->DeployType);
		}
	}
}

void UNxPatchWidget::ClickPatchShutdown()
{
	if (IsPatchReady == true)
	{
		IsPatchReady = false;

		if (GetPatchSubSystem())
		{
			// 패칭 종료.
			GetPatchSubSystem()->ShutdownPatching();
		}
		
		ShowLayout(false);		
	}
}

void UNxPatchWidget::ClickChunking()
{
	if (IsPatchReady == true)
	{
		int32 chunkId = 0;
		
		if (ET_InputChunkId)
		{
			FString textId = ET_InputChunkId->GetText().ToString();

			if (FDefaultValueHelper::ParseInt(textId, chunkId))
			{
				if (GetPatchSubSystem())
				{
					// 청크 다운로드 시도
					if (GetPatchSubSystem()->DownloadSingleChunk(chunkId))
					{
						ShowLayout(true);

						// 업데이트에서 갱신됨.
					}
					else
					{
						FText errorMsg = FText::FromString(TEXT("Patch Failed!"));
						SetErrorMessage(errorMsg);
					}
				}
			}
		}
	}
}

void UNxPatchWidget::UpdatePatch()
{
	if (GetPatchSubSystem())
	{
		const FNxPatchStats& patchStats = GetPatchSubSystem()->GetPatchStatus();

		if (patchStats.TotalFilesToDownload != 0)
		{
			// 다운로드 진행.
			PB_PatchDownload->SetPercent(patchStats.DownloadPercent);

			FString downloadString	= FString::Printf(TEXT("%d/%d MB, %d/%d Files"), 
				patchStats.BytesDownloaded, patchStats.TotalBytesToDownload,
				patchStats.FilesDownloaded, patchStats.TotalFilesToDownload);
			FText   downloadText	= FText::FromString(downloadString);
			TB_PatchText->SetText( downloadText );

		}
		else
		{
			// 다운로드 완료.
			PB_PatchDownload->SetPercent(1.0f);
		}

		// 마운트
		PB_Mount->SetPercent(patchStats.MountPercent);

		FString downloadString = FString::Printf(TEXT("%d/%d Mount"), patchStats.ChunksMounted, patchStats.TotalChunksToMount);
		FText   downloadText = FText::FromString(downloadString);
		TB_MountText->SetText(downloadText);

		// 알림.
		TB_NotifyText->SetText(patchStats.LastError);
	}
}

