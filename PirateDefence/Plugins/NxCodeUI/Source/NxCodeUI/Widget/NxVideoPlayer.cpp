// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxVideoPlayer.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaPlaylist.h"
#include "MediaSource.h"
#include "StreamMediaSource.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MediaSoundComponent.h"
#include "ShaderPipelineCache.h"
#include "IMediaEventSink.h"
#include "Widgets/Images/SImage.h"
#include "Components/PanelSlot.h"
#include "NxCodeUIUtility.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(NxVideoPlayer)

UNxVideoPlayer::UNxVideoPlayer(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	if (nullptr == VideoMaterial)
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> VideoPlayerMaterialFinder(TEXT("/CommonUI/VideoPlayerMaterial"));
		VideoMaterial = VideoPlayerMaterialFinder.Object;
	}
}

void UNxVideoPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (!IsTemplate())
	{
		MediaPlayer = NewObject<UMediaPlayer>(this);
		MediaPlayer->PlayOnOpen = bOption_PlayOnOpen;
		MediaPlayer->Shuffle = bOption_Shuffle;
		MediaPlayer->OnMediaEvent().AddUObject(this, &UNxVideoPlayer::HandleMediaPlayerEvent);

		MediaTexture = NewObject<UMediaTexture>(this);
		MediaTexture->AutoClear = true;
		MediaTexture->SetMediaPlayer(MediaPlayer);
		MediaTexture->UpdateResource();

		if (nullptr == VideoMaterial)
		{
			VideoMaterial = (UMaterial*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/CommonUI/VideoPlayerMaterial"), NULL, LOAD_None, NULL);
		}

		UMaterialInstanceDynamic* VideoMID = UMaterialInstanceDynamic::Create(VideoMaterial, this);
		VideoMID->SetTextureParameterValue(TEXT("MediaTexture"), MediaTexture);
		VideoBrush.SetResourceObject(VideoMID);

		SoundComponent = NewObject<UMediaSoundComponent>(this);
		SoundComponent->Channels = EMediaSoundChannels::Stereo;
		SoundComponent->bIsUISound = true;

		SoundComponent->SetMediaPlayer(MediaPlayer);
		SoundComponent->Initialize();
		SoundComponent->UpdatePlayer();
	}
}

void UNxVideoPlayer::SetVideo(UMediaSource* NewVideo)
{
	if (MediaPlayer->GetPlaylist()->Get(0) != NewVideo)
	{
		//MediaPlayer->Close();
		MediaPlayer->OpenSource(NewVideo);
	}
}

void UNxVideoPlayer::SetStreamUrl(FString& Url)
{
	auto mediaSource = NewObject<UStreamMediaSource>(this);
	mediaSource->StreamUrl = Url;

	SetVideo(mediaSource);
}

void UNxVideoPlayer::Seek(float PlaybackTime)
{
	MediaPlayer->Seek(FTimespan::FromSeconds(PlaybackTime));
}

void UNxVideoPlayer::Close()
{
	MediaPlayer->Close();
	SoundComponent->Stop();

	OnPlaybackComplete().Broadcast();
}

void UNxVideoPlayer::SetPlaybackRate(float PlaybackRate)
{
	MediaPlayer->SetRate(PlaybackRate);
}

void UNxVideoPlayer::SetLooping(bool bShouldLoopPlayback)
{
	MediaPlayer->SetLooping(bShouldLoopPlayback);
}

void UNxVideoPlayer::SetIsMuted(bool bInIsMuted)
{
	bIsMuted = bInIsMuted;
	if (bIsMuted)
	{
		SoundComponent->Stop();
	}
	else if (IsPlaying())
	{
		SoundComponent->Start();
	}
}

void UNxVideoPlayer::Play()
{
	SetPlaybackRate(1.f);
}

void UNxVideoPlayer::Reverse()
{
	SetPlaybackRate(-1.f);
}

void UNxVideoPlayer::Pause()
{
	MediaPlayer->Pause();
}

void UNxVideoPlayer::PlayFromStart()
{
	MediaPlayer->Rewind();
	Play();
}

float UNxVideoPlayer::GetVideoDuration() const
{
	return MediaPlayer->GetDuration().GetTotalSeconds();
}

float UNxVideoPlayer::GetPlaybackTime() const
{
	return MediaPlayer->GetTime().GetTotalSeconds();
}

float UNxVideoPlayer::GetPlaybackRate() const
{
	return MediaPlayer->GetRate();
}

bool UNxVideoPlayer::IsLooping() const
{
	return MediaPlayer->IsLooping();
}

bool UNxVideoPlayer::IsPaused() const
{
	return MediaPlayer->IsPaused();
}

bool UNxVideoPlayer::IsPlaying() const
{
	return MediaPlayer->IsPlaying();
}

void UNxVideoPlayer::SetOriginSize()
{
	const int64 ResolutionWidth  = Videos[0]->GetMediaOption(TEXT("ResolutionWidth"), (int64)1280);
	const int64 ResolutionHeight = Videos[0]->GetMediaOption(TEXT("ResolutionHeight"), (int64)720);
	FVector2D size(ResolutionWidth, ResolutionHeight);

	Nx_UtilityUI::SetSize(this, size);
}

TSharedRef<SWidget> UNxVideoPlayer::RebuildWidget()
{
	return SAssignNew(MyImage, SImage)
		.Image(&VideoBrush);
}

void UNxVideoPlayer::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (Videos.Num() > 0)
	{
		UMediaPlaylist* playList = NewObject<UMediaPlaylist>();
		for (auto video : Videos)
		{
			playList->Add(video);
			MediaPlayer->OpenPlaylist(playList);
		}
	}

	SetLooping(bOption_Loop);
	SetIsMuted(bOption_Mute);
	if (true == bOption_OriginalSize)
		SetOriginSize();
}

void UNxVideoPlayer::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyImage.Reset();
}

void UNxVideoPlayer::PlayInternal() const
{
	if (MediaPlayer->IsReady() && !MediaPlayer->IsPlaying())
	{
		MediaPlayer->Play();
	}
}

void UNxVideoPlayer::HandleMediaPlayerEvent(EMediaEvent EventType)
{
	switch (EventType)
	{
	case EMediaEvent::MediaClosed:
		//Close();
		break;
	case EMediaEvent::PlaybackEndReached:
		if (!IsLooping())
		{
			SoundComponent->Stop();
			OnPlaybackComplete().Broadcast();
		}
		break;
	case EMediaEvent::PlaybackResumed:
		if (!bIsMuted)
		{
			SoundComponent->Start();
		}
		if (MyImage)
		{
			MyImage->RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateUObject(this, &UNxVideoPlayer::HandlePlaybackTick));
		}
		break;
	case EMediaEvent::PlaybackSuspended:
		SoundComponent->Stop();
		break;
	}
}

void UNxVideoPlayer::PlaybackTick(double InCurrentTime, float InDeltaTime)
{
	if (!bIsMuted)
	{
		SoundComponent->UpdatePlayer();
	}
}

EActiveTimerReturnType UNxVideoPlayer::HandlePlaybackTick(double InCurrentTime, float InDeltaTime)
{
	PlaybackTick(InCurrentTime, InDeltaTime);
	return MediaPlayer->IsPlaying() ? EActiveTimerReturnType::Continue : EActiveTimerReturnType::Stop;
}
