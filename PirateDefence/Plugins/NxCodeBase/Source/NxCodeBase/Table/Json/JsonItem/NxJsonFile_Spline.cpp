// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Json/JsonItem/NxJsonFile_Spline.h"

#include "Misc/Paths.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "Kismet/KismetStringLibrary.h"
#include "NxBaseLog.h"

//------------------------------------------------------------------------------------ json 파일 심볼 Tag
namespace Nx_JsonTAG
{
	const FString Json_Spline_Data		= TEXT("datas");
	const FString Json_Spline_Key		= TEXT("key");
	const FString Json_Spline_Splines	= TEXT("splines");
	const FString Json_Spline_Name		= TEXT("name");
	const FString Json_Spline_Points	= TEXT("points");
	
	const FString Json_Spline_X			= TEXT("x");
	const FString Json_Spline_Y			= TEXT("y");
	const FString Json_Spline_Z			= TEXT("z");
}


//------------------------------------------------------------------------------------ Data
void FJsonData_SplineUnit::PrintLog()
{
	FString printString = FString::Printf(TEXT("Seq:%d, Name:%s, PointCnt[%d]"), SeqID, *Name, Points.Num());
	if(Points.Num() > 0)
		printString = FString::Printf(TEXT("%s - first[%0.2f, %0.2f %0.2f]"), *printString, Points[0].X, Points[0].Y, Points[0].Z);

	NxPrintLog(LogBase, TEXT("%s"), *printString);
}

void FJsonData_SplineUnit::CalcLocAndRot()
{
	// Point 들의 중심점 얻기

	// 회전은 Unit
}

void FJsonData_SplineSet::PrintLog()
{
	NxPrintLog(LogBase, TEXT("Seq:%d, Key:%d"), SeqID, KeyID);
	for( auto& unit : ArrUnit )
		unit.PrintLog();
}



//------------------------------------------------------------------------------------ Class
UNxJsonFile_Spline::UNxJsonFile_Spline()
{
}

bool UNxJsonFile_Spline::Parsing(FString& InFilename)
{
	if (InFilename.IsEmpty())
		return false;

	if (OpenJsonFile(InFilename) == false)
		return false;

	SplineData.Clear();
	SplineData.Filename = InFilename;

	FJsonData_SplineUnit splineUnit;
	FJsonData_SplineSet	 splineSet;

	// Parsing Test...
	const FJsonObject* JsonRoot = GetJsonObject();
	const TArray<TSharedPtr<FJsonValue>>* dataArray;

	// 해당 필드를 모두 가져온다.
	if (JsonRoot->TryGetArrayField(Nx_JsonTAG::Json_Spline_Data, dataArray))
	{
		NxPrintLog(LogBase, TEXT("Json[%s] DataCount[%d]"), *InFilename, dataArray->Num());

		for (int di = 0; di < dataArray->Num(); ++di)
		{
			splineSet.Clear();
			{
				TSharedPtr<FJsonValue>  dataSeq			= (*dataArray)[di];
				TSharedPtr<FJsonObject> dataSeqObject	= dataSeq->AsObject();

				// Parse : Nx_JsonTAG::Json_Spline_Key
				FString keyID = dataSeqObject->GetStringField(Nx_JsonTAG::Json_Spline_Key);

				splineSet.SeqID = di;
				splineSet.KeyID = UKismetStringLibrary::Conv_StringToInt(keyID);

				// Parse : Nx_JsonTAG::Json_Spline_Splines
				const TArray<TSharedPtr<FJsonValue>>* splineArray;
				if (dataSeqObject->TryGetArrayField(Nx_JsonTAG::Json_Spline_Splines, splineArray))
				{
					for (int si = 0; si < splineArray->Num(); ++si)
					{
						splineUnit.Clear();
						{
							TSharedPtr<FJsonValue>  dataSpline = (*splineArray)[si];
							TSharedPtr<FJsonObject> dataSplineObject = dataSpline->AsObject();

							splineUnit.SeqID = si;

							// name 파싱
							FString splineName = dataSplineObject->GetStringField(Nx_JsonTAG::Json_Spline_Name);
							splineUnit.Name = splineName;

							// points 파싱
							const TArray<TSharedPtr<FJsonValue>>* pointArray;
							if (dataSplineObject->TryGetArrayField(Nx_JsonTAG::Json_Spline_Points, pointArray))
							{
								for (int pi = 0; pi < pointArray->Num(); ++pi)
								{
									TSharedPtr<FJsonValue>  dataPoint = (*pointArray)[pi];
									TSharedPtr<FJsonObject> dataPointObject = dataPoint->AsObject();

									// x,y,z 값 얻기
									FVector Point;
									Point.X = dataPointObject->GetNumberField(Nx_JsonTAG::Json_Spline_X);
									Point.Y = dataPointObject->GetNumberField(Nx_JsonTAG::Json_Spline_Y);
									Point.Z = dataPointObject->GetNumberField(Nx_JsonTAG::Json_Spline_Z);
									splineUnit.Points.Add(Point);
								}
							}
						}
						splineSet.ArrUnit.Add(splineUnit);
					}
				}
			}
			SplineData.ArrSet.Add(splineSet);
		}
	}

	Close();
	return true;
}

// Binary 저장 (지원 안함)
bool UNxJsonFile_Spline::SaveBinary(FString filename)
{
	return false;
}

// Binary 읽기 (지원 안함)
bool UNxJsonFile_Spline::LoadBinary(FString filename)
{
	return false;
}

