// Fill out your copyright notice in the Description page of Project Settings.


#include "DetailCustomization/NxDetailCustomizationActor.h"

#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"

#include "Landscape.h"						// Landscape
#include "LandscapeMeshProxyActor.h"
#include "LandscapeStreamingProxy.h"
#include "LocationVolume.h"					// Engine


// Sets default values
ANxDetailCustomizationActor::ANxDetailCustomizationActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Scene->SetMobility(EComponentMobility::Static);
	SetRootComponent(Scene);

	if (GetRootComponent())
	{
		

#if WITH_EDITOR
		BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
		if (BillboardComponent != nullptr)
		{
			BillboardComponent->SetupAttachment(GetRootComponent());
			BillboardComponent->SetMobility(EComponentMobility::Static);
			BillboardComponent->SetRelativeLocation(FVector(0, 0, 0.0f));
			BillboardComponent->SetEditorScale(1.0f);
			BillboardComponent->bHiddenInGame = true;

			static ConstructorHelpers::FObjectFinder<UTexture2D> iconTexture(TEXT("/NxCodeBaseEditor/Texture/Icon/S_VxActor.S_VxActor"));
			if (iconTexture.Succeeded())
			{
				BillboardComponent->SetSprite(iconTexture.Object);
			}
		}
#endif 
	}

#if WITH_EDITOR
	this->bIsSpatiallyLoaded = false;
	this->SetRuntimeGrid(FName("None"));	// "Editor" 처럼 임의의 그리드 이름을 넣을 경우, WorldSetting에 Grid 가 존재해야 함.
#endif 
}

void ANxDetailCustomizationActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

// Called when the game starts or when spawned
void ANxDetailCustomizationActor::BeginPlay()
{
	Super::BeginPlay();
}

void ANxDetailCustomizationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ANxDetailCustomizationActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
void ANxDetailCustomizationActor::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName propertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;

	if (propertyName == GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomComboList))
	{
		// ...
	}

	Super::PostEditChangeProperty(e);
}

void ANxDetailCustomizationActor::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e)
{
	Super::PostEditChangeChainProperty(e);

	FEditPropertyChain::TDoubleLinkedListNode* nextNode = e.PropertyChain.GetHead()->GetNextNode();
	if (!nextNode)
		return;

	FEditPropertyChain::TDoubleLinkedListNode* dataNode = nextNode->GetNextNode();
	if (!dataNode)
		return;

}
#endif // WITH_EDITOR

