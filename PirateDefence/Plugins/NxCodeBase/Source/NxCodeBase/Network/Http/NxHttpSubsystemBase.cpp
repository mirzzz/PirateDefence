// Fill out your copyright notice in the Description page of Project Settings.


#include "NxHttpSubsystemBase.h"
#include "NxHttpSessionClientBase.h"
#include "NxBaseLog.h"

void UNxHttpSubsystemBase::Post(const FString& InBaseURL, const FString& InSubURL, const FString& InContent, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete)
{
	FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
	UNxHttpSessionClientBase* HttpSessionClient = Create(InBaseURL, MoveTemp(OnRequestComplete), OnRequestCompleteDelegate);
	HttpSessionClient->Post(InSubURL, InContent, OnRequestCompleteDelegate);
}

void UNxHttpSubsystemBase::Get(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete)
{
	FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
	UNxHttpSessionClientBase* HttpSessionClient = Create(InBaseURL, MoveTemp(OnRequestComplete), OnRequestCompleteDelegate);
	HttpSessionClient->Get(InSubURL, OnRequestCompleteDelegate);
}

void UNxHttpSubsystemBase::Put(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete)
{
	FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
	UNxHttpSessionClientBase* HttpSessionClient = Create(InBaseURL, MoveTemp(OnRequestComplete), OnRequestCompleteDelegate);
	HttpSessionClient->Put(InSubURL, OnRequestCompleteDelegate);
}

void UNxHttpSubsystemBase::Delete(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete)
{
	FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
	UNxHttpSessionClientBase* HttpSessionClient = Create(InBaseURL, MoveTemp(OnRequestComplete), OnRequestCompleteDelegate);
	HttpSessionClient->Delete(InSubURL, OnRequestCompleteDelegate);
}

void UNxHttpSubsystemBase::Patch(const FString& InBaseURL, const FString& InSubURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete)
{
	FHttpRequestCompleteDelegate OnRequestCompleteDelegate;
	UNxHttpSessionClientBase* HttpSessionClient = Create(InBaseURL, MoveTemp(OnRequestComplete), OnRequestCompleteDelegate);
	HttpSessionClient->Patch(InSubURL, OnRequestCompleteDelegate);
}

UNxHttpSessionClientBase* UNxHttpSubsystemBase::Create(const FString& InBaseURL, TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnRequestComplete, FHttpRequestCompleteDelegate& OutOnRequestComplete)
{
	UNxHttpSessionClientBase* HttpSessionClient = NewObject<UNxHttpSessionClientBase>();
	HttpSessionClient->Init(InBaseURL);
	HttpSessionClient->SetRequestComplete(MoveTemp(OnRequestComplete));
	HttpReqs.Add(HttpSessionClient);

	OutOnRequestComplete.BindLambda([this, HttpSessionClient](FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success) {
		HttpSessionClient->OnRequestComplete(Request, Response, Success);
		int32 Pos = HttpReqs.Find(HttpSessionClient);
		if (INDEX_NONE != Pos)
			HttpReqs.RemoveAt(Pos);
		});

	return HttpSessionClient;
}