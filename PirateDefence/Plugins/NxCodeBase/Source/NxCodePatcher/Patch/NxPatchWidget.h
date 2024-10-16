// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NxPatchWidget.generated.h"

class UNxPatchSubsystem;

/**
 * Patch 지원 UI Layout
 */ 
UCLASS()
class NXCODEPATCHER_API UNxPatchWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual bool Initialize() override;				// CDO
	virtual void RemoveFromParent() override;		// CDO

	// 초기화 설정
	virtual void NativeOnInitialized() override;

	// AddToViewport() 함수가 호출될 때 UI위젯을 생성/소멸 하기 위해 호출됨.
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void ShowLayout(bool show = true);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void ShowMenu(bool show = true);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void SetMessage(FText msg);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void SetErrorMessage(FText msg);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void ResetProgressBar(float percent = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void SetPatchProgressBar(float percent, FString text);

	UFUNCTION(BlueprintCallable, Category = "Patcher")
	void SetMountProgressBar(float percent, FString text);


	UFUNCTION()
	void ClickStartMap();
	UFUNCTION()
	void ClickPatchInitialize();
	UFUNCTION()
	void ClickPatchShutdown();
	UFUNCTION()
	void ClickChunking();

	void UpdatePatch();

	void SetPatchReady(bool flag) { IsPatchReady = flag; }

	class UNxPatchSubsystem*		GetPatchSubSystem();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patcher")
	bool IsPatchReady = false;	

public:

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UCanvasPanel>	CP_Layout;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UOverlay>		OL_Menu;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UOverlay>		OL_Status;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UButton>		BT_StartMap;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UButton>		BT_PatchInitialize;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UButton>		BT_PatchShutdown;
	
	// 단일 Pak 
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UButton>		BT_Chunking;

	// 단일 ChunkId
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UEditableText>	ET_InputChunkId;

	// Patch
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UProgressBar>	PB_PatchDownload;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UTextBlock>	TB_PatchText;

	// Mount
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UProgressBar>	PB_Mount;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UTextBlock>	TB_MountText;
	
	// 패치 알림 메세지
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<class UTextBlock>	TB_NotifyText;
};
