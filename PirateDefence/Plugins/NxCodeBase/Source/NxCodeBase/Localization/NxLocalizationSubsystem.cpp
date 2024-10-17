// Fill out your copyright notice in the Description page of Project Settings.


#include "NxLocalizationSubsystem.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetTextLibrary.h"

#include "NxGameInstanceBase.h"
#include "NxTableSubsystem.h"
#include "NxBaseLog.h"

/*static*/ UNxLocalizationSubsystem* UNxLocalizationSubsystem::Get()
{
	return UNxGameInstanceBase::GetGameInstanceSubsystem<UNxLocalizationSubsystem>();
}

const FString UNxLocalizationSubsystem::Code_EN = TEXT("en");
const FString UNxLocalizationSubsystem::Code_KR = TEXT("ko-KR");

void UNxLocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//FText findTest = GetText_UI(TEXT("UI_FrontEnd_Menu_Setting_Title"));
	//VxLogParam(TEXT("UVxLocalizationSubsystem::Test - %s"), *(findTest.ToString()));
}

FString UNxLocalizationSubsystem::GetCulture()
{
	return UKismetInternationalizationLibrary::GetCurrentCulture();
}

void UNxLocalizationSubsystem::SetCulture(const FString& Language)
{
	UKismetInternationalizationLibrary::SetCurrentCulture(Language);
}

FText UNxLocalizationSubsystem::GetFromStringTable(E_StringTable InEnumStringTable, const FString& InKey)
{
	FName tagleId = UNxTableSubsystem::Get()->GetStringTableId(InEnumStringTable);
	return GetFromStringTable(tagleId, InKey);
}

FText UNxLocalizationSubsystem::GetFromStringTable(const FName& InTableId, const FString& InKey)
{
	FText Find = FText::FromStringTable(InTableId, InKey);
	if (false == Find.IsEmpty())
		return Find;
	else
		return FText::FromString(InKey);
}

FText UNxLocalizationSubsystem::GetText(const FTextKey& Namespace, FTextKey Key)
{
	FText Find;
	if (!FText::FindText(Namespace, Key, Find))
		return FText::FromString(Key.GetChars());

	return Find;
}