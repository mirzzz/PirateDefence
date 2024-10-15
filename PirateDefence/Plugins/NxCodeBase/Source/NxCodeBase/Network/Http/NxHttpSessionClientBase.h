// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "PlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "NxHttpSessionClientBase.generated.h"

//https://wiki.unrealengine.com/Http-requests
//USTRUCT()
//struct FRequest_Login
//{
//	GENERATED_BODY()
//	FRequest_Login() {}
//
//	UPROPERTY() FString Email;
//	UPROPERTY() FString PassWord;
//};
//
//USTRUCT()
//struct FResponse_Login
//{
//	GENERATED_BODY()
//	FResponse_Login() {}
//
//	UPROPERTY() int ID;
//	UPROPERTY() FString Name;
//	UPROPERTY() FString Hash;
//};

UCLASS()
class NXCODEBASE_API UNxHttpSessionClientBase : public UObject
{
	GENERATED_BODY()

public:
	UNxHttpSessionClientBase();

	virtual bool Init(const FString& InBaseURL);
	void SetRequestComplete(TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> InOnRequestComplete);

	template <typename TRequest>
	void PostStruct(const FString& InSubRoute, TRequest& InRequest, FHttpRequestCompleteDelegate& InOnRequestComplete)
	{
		FString Content;
		GetStruct2Json<TRequest>(InRequest, Content);

		Post(InSubRoute, Content, InOnRequestComplete);
	}
	void Post(const FString& InSubRoute, const FString& InContent, FHttpRequestCompleteDelegate& InOnRequestComplete);
	void Get(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete);
	void Put(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete);
	void Delete(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete);
	void Patch(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete);

	//

protected:
	FHttpRequestRef Post(FString SubRoute, FString Content);
	FHttpRequestRef Get(FString SubRoute);
	FHttpRequestRef Put(FString SubRoute);
	FHttpRequestRef Delete(FString SubRoute);
	FHttpRequestRef Patch(FString SubRoute);

	FHttpRequestRef CreateRequest(FString SubRoute, FString Verb);
	void SetRequestHeaders(FHttpRequestRef RequestRef);
	bool CheckResponse(FHttpResponsePtr ResponsePtr, bool bWasSuccessful);

public:
	template <typename StructType>
	static void GetStruct2Json(StructType FilledStruct, FString& StringOutput)
	{
		FJsonObjectConverter::UStructToJsonObjectString(StructType::StaticStruct(), &FilledStruct, StringOutput, 0, 0);
	}

	template <typename StructType>
	static void GetJson2Struct(FHttpResponsePtr Response, StructType& StructOutput)
	{
		StructType StructData;
		FString JsonString = Response->GetContentAsString();
		FJsonObjectConverter::JsonObjectStringToUStruct<StructType>(JsonString, &StructOutput, 0, 0);
	}

public:
	TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete = [](FHttpRequestPtr, FHttpResponsePtr, bool) {};

protected:
	FHttpModule* HttpModule;
	FString		 BaseURL = "http://localhost:5000/api/";
};
