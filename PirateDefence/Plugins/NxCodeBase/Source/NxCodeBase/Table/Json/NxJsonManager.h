// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxTableEnum.h"
#include "NxJsonManager.generated.h"

/**
	Json File 관리자
		> Build.cs 에  "Json" 추가 필요
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxJsonManager : public UObject
{
	GENERATED_BODY()
	
public:
	UNxJsonManager();
		
#if WITH_EDITOR
	// 에디터에서 json 파일리스트 열기
	void RegistDataJsonFormDialog();
#endif 

	// 데이터 json 파일 등록
	bool RegistDataJson(E_JsonData jsonType, FString filename);

	template <typename T>
	T* GetJsonData(E_JsonData jsonType)
	{
		return static_cast<T*>(GetJson(jsonType)->GetData());
	}

	// 데이터 접근
	UFUNCTION(BlueprintCallable, Category = Json)
	class UNxJsonParser* GetJson(E_JsonData jsonType);			

	UFUNCTION(BlueprintCallable, Category = Json)
	bool HasJson(const E_JsonData jsonType);

protected:

	// DataTable Registry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Json, Meta = (AllowPrivateAccess = true))
	TMap<E_JsonData, TObjectPtr<class UNxJsonParser> > JsonTables;
	
};
 