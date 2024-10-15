// Fill out your copyright notice in the Description page of Project Settings.

#include "NxHttpSessionClientBase.h"

UNxHttpSessionClientBase::UNxHttpSessionClientBase()
{
}

bool UNxHttpSessionClientBase::Init(const FString& InBaseURL)
{
	HttpModule = &FHttpModule::Get();
	BaseURL = InBaseURL;

	return true;
}

void UNxHttpSessionClientBase::SetRequestComplete(TUniqueFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> InOnRequestComplete)
{
	OnRequestComplete = MoveTemp(InOnRequestComplete);
}

void UNxHttpSessionClientBase::Post(const FString& InSubRoute, const FString& InContent, FHttpRequestCompleteDelegate& InOnRequestComplete)
{
	FHttpRequestRef RequestRef = Post(InSubRoute, InContent);
	//RequestRef->OnProcessRequestComplete().BindUObject(this, &OnRequestComplete);
	RequestRef->OnProcessRequestComplete() = InOnRequestComplete;
	RequestRef->ProcessRequest();
}

void UNxHttpSessionClientBase::Get(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete)
{
	FHttpRequestRef RequestRef = Get(InSubRoute);
	RequestRef->OnProcessRequestComplete() = InOnRequestComplete;
	RequestRef->ProcessRequest();
}

void UNxHttpSessionClientBase::Put(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete)
{
	FHttpRequestRef RequestRef = Put(InSubRoute);
	RequestRef->OnProcessRequestComplete() = InOnRequestComplete;
	RequestRef->ProcessRequest();
}

void UNxHttpSessionClientBase::Delete(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete)
{
	FHttpRequestRef RequestRef = Delete(InSubRoute);
	RequestRef->OnProcessRequestComplete() = InOnRequestComplete;
	RequestRef->ProcessRequest();
}

void UNxHttpSessionClientBase::Patch(const FString& InSubRoute, FHttpRequestCompleteDelegate& InOnRequestComplete)
{
	FHttpRequestRef RequestRef = Patch(InSubRoute);
	RequestRef->OnProcessRequestComplete() = InOnRequestComplete;
	RequestRef->ProcessRequest();
}

//
FHttpRequestRef UNxHttpSessionClientBase::Post(FString SubRoute, FString Content)
{
	FHttpRequestRef RequestRef = CreateRequest(SubRoute, "POST");
	RequestRef->SetContentAsString(Content);
	return RequestRef;
}

FHttpRequestRef UNxHttpSessionClientBase::Get(FString SubRoute)
{
	FHttpRequestRef RequestRef = CreateRequest(SubRoute, "GET");
	return RequestRef;
}

FHttpRequestRef UNxHttpSessionClientBase::Put(FString SubRoute)
{
	FHttpRequestRef RequestRef = CreateRequest(SubRoute, "PUT");
	return RequestRef;
}

FHttpRequestRef UNxHttpSessionClientBase::Delete(FString SubRoute)
{
	FHttpRequestRef RequestRef = CreateRequest(SubRoute, "DELETE");
	return RequestRef;

}

FHttpRequestRef UNxHttpSessionClientBase::Patch(FString SubRoute)
{
	FHttpRequestRef RequestRef = CreateRequest(SubRoute, "PATCH");
	return RequestRef;
}

FHttpRequestRef UNxHttpSessionClientBase::CreateRequest(FString SubRoute, FString Verb)
{
	FHttpRequestRef RequestRef = HttpModule->CreateRequest();
	RequestRef->SetURL(BaseURL + SubRoute);
	RequestRef->SetVerb(Verb);
	SetRequestHeaders(RequestRef);
	return RequestRef;
}

void UNxHttpSessionClientBase::SetRequestHeaders(FHttpRequestRef RequestRef)
{
	RequestRef->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	RequestRef->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	RequestRef->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

bool UNxHttpSessionClientBase::CheckResponse(FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
{
	if (!ResponsePtr.IsValid() || !bWasSuccessful) return false;

	int32 ResponseCode = ResponsePtr->GetResponseCode();

	if (EHttpResponseCodes::IsOk(ResponseCode))
	{
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpModule::CheckResponse Error Code : %d"), ResponseCode);
		return false;
	}
}

//void UVxHttpSessionClient::Login(FRequest_Login LoginCredentials)
//{
//	FString ContentJsonString;
//	GetStruct2Json<FRequest_Login>(LoginCredentials, ContentJsonString);
//
//	FHttpRequestRef Request = Post("user/login", ContentJsonString);
//	Request->OnProcessRequestComplete().BindUObject(this, &UVxHttpSessionClient::LoginResponse);
//	Send(Request);
//}
//
//void UVxHttpSessionClient::LoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
//{
//	if (!CheckResponse(Response, bWasSuccessful)) return;
//
//	FResponse_Login LoginResponse;
//	GetJson2Struct<FResponse_Login>(Response, LoginResponse);
//
//	UE_LOG(LogTemp, Warning, TEXT("Id is: %d"), LoginResponse.ID);
//	UE_LOG(LogTemp, Warning, TEXT("Name is: %s"), *LoginResponse.Name);
//}

//void UVxHttpSessionClient::SetAuthorizationHash(FString Hash, FHttpRequestRef& Request)
//{
//	Request->SetHeader(AuthorizationHeader, Hash);
//}

