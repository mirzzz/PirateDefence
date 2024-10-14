// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Engine.h"
//#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"				// GetPlayerCharacter() 지원.
#include "GameFramework/Actor.h"
#include "NxDetailCustomizationActor.generated.h"

/*
* DetailCustomization Template Actor - 사용자 정의 Detail창 사용 예.
*/ 
UCLASS(BlueprintType, Blueprintable)
class NXCODEBASEEDITOR_API ANxDetailCustomizationActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANxDetailCustomizationActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// 에디터에서 이동 및 속성값을 편집할때 호출됨.
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
#if WITH_EDITORONLY_DATA
	
	// 빌보드 컴포넌트
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Base)
	TObjectPtr<class UBillboardComponent> BillboardComponent = nullptr;

#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e) override;
#endif 

public:

	// 해당 변수 커스텀 적용시킴 > FNxDetailCustomizationSample::CustomizeDetails
	UPROPERTY(EditAnywhere, Category = Base)
	FString CustomString;

	UPROPERTY(EditAnywhere, Category = Base)
	bool	CustomBool;

	UPROPERTY(EditAnywhere, Category = Base)
	TArray<FString> CustomComboList;

	UPROPERTY(VisibleAnywhere, Category = Base)
	FString			CustomComboChoose;
};
