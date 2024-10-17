// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonBorder.h"
#include "NxImageBorder.generated.h"

/**
 * 
 */
UCLASS(Config = NxCodeUI, DefaultConfig, ClassGroup = UI, meta = (Category = CodeUI, DisplayName = "NxImageBorder"))
class NXCODEUI_API UNxImageBorder : public UCommonBorder
{
	GENERATED_BODY()
	
public:
	virtual void OnWidgetRebuilt() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nx|Border", meta = (AllowPrivateAccess = "true", DisplayThumbnail = "true", DisplayName = "Image", AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TObjectPtr<UObject> ResourceObject;

	//UPROPERTY(EditAnywhere, Category = "Nx|Border")
	//UTexture2D* Texture;
};