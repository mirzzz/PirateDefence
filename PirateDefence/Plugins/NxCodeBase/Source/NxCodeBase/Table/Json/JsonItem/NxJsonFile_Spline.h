// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Table/Json/NxJsonParser.h"
#include "NxJsonFile_Spline.generated.h"

// 단일 Spline 정보
class FJsonData_SplineUnit
{
public:
	
	int32				SeqID;
	FString				Name;	
	FVector				Loc;
	FRotator			Rot;
	TArray<FVector>		Points;

	FJsonData_SplineUnit() { Clear(); }

	void Clear()
	{
		SeqID = 0;
		Name	= FString(TEXT("None"));
		Loc		= FVector::ZeroVector;
		Rot		= FRotator::ZeroRotator;
		Points.Empty();
	}

	void PrintLog();	
	void CalcLocAndRot();	
};

// Spline 묶음 정보
class FJsonData_SplineSet
{
public:
	int32 SeqID;
	int32 KeyID;
	
	TArray<FJsonData_SplineUnit> ArrUnit;

	void Clear()
	{
		SeqID = 0;
		KeyID = 0;
		ArrUnit.Empty();
	}

	void PrintLog();
};


// 파일 전체 정보
class FJsonData_Spline : public FNxJsonData
{
public:
	FString							Filename;
	TArray<FJsonData_SplineSet>		ArrSet;

	void Clear()
	{
		Filename = TEXT("None");
		ArrSet.Empty();
	}
};


/*
 * spline.json 파일을 읽어드릴 parser.
 */
UCLASS()
class NXCODEBASE_API UNxJsonFile_Spline : public UNxJsonParser
{
	GENERATED_BODY()

public:
	UNxJsonFile_Spline();
	
	// 수집된 정보 접근 지원.
	virtual bool	     Parsing(FString& filename) override;
	virtual FNxJsonData* GetData() override { return &SplineData; }
	
protected:
	virtual bool	SaveBinary(FString filename);
	virtual bool	LoadBinary(FString filename);

protected:
	FJsonData_Spline SplineData;
};
