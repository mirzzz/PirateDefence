// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "NxHttpSubsystemBase.generated.h"

class UNxHttpSessionClientBase;


/**
 *
 */
UCLASS()
class NXCODEBASE_API UNxHttpSubsystemBase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Post(const FString& InBaseURL, const FString& InSubURL, const FString& InContent,  TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete);
	void Get(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete);
	void Put(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete);
	void Delete(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete);
	void Patch(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete);

protected:
	UNxHttpSessionClientBase* Create(const FString& InBaseURL, 
		TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete, FHttpRequestCompleteDelegate& OutOnRequestComplete);

protected:
	TArray<UNxHttpSessionClientBase*> HttpReqs;
};
