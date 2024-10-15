#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NxTable_Template.generated.h"
 
UENUM(BlueprintType)
enum class E_TemplateOrder : uint8
{
	Enum_A,
	Enum_B,
	Enum_C,
	Enum_D,
	Enum_E,
	Enum_F,
	Enum_Max,
};

/**
	템블릿 테스트 csv 파일 레코드
		- DataTableUtils::IsSupportedTableProperty()

	데이터 테이블의 위한 Struct 구성
		- Excel 파일과 구성을 맞춰서 생성해줘야 함. (Format 구성 : 액셀의 Field 이름과 똑같이 구성 필요)
		- C++ 창에 노출 안됨.
*/

// DataTable Template
USTRUCT(BlueprintType)
struct FNxTable_Template : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	E_TemplateOrder TempEnum = E_TemplateOrder::Enum_A;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	FName TempString = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	int32 TempInt   = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	int64 TempInt64 = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	float TempFloat = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	FVector TempVector = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	FRotator TempRotator = FRotator::ZeroRotator;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	FName TempParsing = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	TSoftObjectPtr<UTexture> TempTexture;		// 지연 로드 에셋 연결

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	TSoftObjectPtr<UDataTable> TempTable;		// 지연 로드 애셋 연결

	// TArray (Value,Value,Value)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	TArray<FString> TempArray;

	// TMap ((Key,Value),(Key,Value))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Table)
	TMap<int32, FString> TempMap;
	
	// 데이터 Import 후 처리 지원.
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override {};

	// 생성자
	FNxTable_Template() {}

	// 할당자
	FNxTable_Template(FNxTable_Template const& Other)
	{
		*this = Other;
	}
};


