// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NxBaseEnum.generated.h"
 
// 개발용 열거형 변수 선언
UENUM(BlueprintType)
enum class E_DevelopmentType : uint8
{
	None    = 0		UMETA(DisplayName = "None"),
	Local	= 1		UMETA(DisplayName = "Local 버전 "),
	Dev		= 2		UMETA(DisplayName = "Dev 버전"),
	QA		= 3		UMETA(DisplayName = "QA 버전"),
	Live	= 4		UMETA(DisplayName = "Live 버전"),
};


namespace Nx_UtilityEnum
{
	// Enum <> String(Name) 변환 : UENUM(BlueprintType) 일경우 지원.
	//-----------------------------------------------------------------------------

	// EnumValue To FString
	template<typename T>
	static FORCEINLINE FString EnumToString(const T value)
	{
		const UEnum* EnumType = StaticEnum<T>();
		ensure(EnumType);
		return *(EnumType ? EnumType->GetNameStringByIndex(static_cast<uint8>(value)) : "Invalid");
	}

	// EnumValue To FName
	template<typename T>
	static FORCEINLINE FName EnumToName(const T Index)
	{
		const UEnum* EnumType = StaticEnum<T>();
		ensure(EnumType);
		return EnumType->GetNameByIndex(static_cast<int32>(Index));
	}

	// EnumValue To FText
	template<typename T>
	static FORCEINLINE FText EnumToText(const T Index)
	{
		return FText::FromString(EnumToString<T>(Index));
	}

	// EnumValue To DisplayName, UMETA(DisplayName = "Value")에서 Value 값 반환
	template<typename T>
	static FORCEINLINE FString EnumToDisplayName(const T value)
	{
		const UEnum* EnumType = StaticEnum<T>();
		return *(EnumType ? EnumType->GetDisplayNameTextByIndex(static_cast<uint8>(value)).ToString() : TEXT("Invalid"));
	}

	// FName To EnumValue
	template <typename T>
	static FORCEINLINE T NameToEnum(const FName& value)
	{
		T enumState = (T)0;
		const UEnum* EnumType = StaticEnum<T>();
		if (EnumType != nullptr)
		{
			int32 index = EnumType->GetIndexByName(value);
			enumState = (T)static_cast<uint8>(index);
		}
		return enumState;
	}

	// FString To EnumValue
	template <typename T>
	static FORCEINLINE T StringToEnum(const FString& value)
	{
		T enumState = (T)0;
		const UEnum* EnumType = StaticEnum<T>();
		if (EnumType != nullptr)
		{
			int32 index = EnumType->GetIndexByNameString(value);
			enumState = (T)static_cast<uint8>(index);
		}
		return enumState;
	}

	// enum 항목 중 from 항목으로 부터 offset거리에 있는 항목을 얻을 때 사용한다.
	template<typename T>
	FORCEINLINE T GetEnumToOffset(const T from, int32 offset)
	{
		return (T)((int)from + offset);
	}

	// from 항목으로부터 enum 항목까지의 offset거리를 구한다.
	template<typename T>
	FORCEINLINE int32 GetOffsetToEnum(const T from, const T to)
	{
		int32 offset = ((int)to - (int)from);
		return offset >= 0 ? offset : offset * -1;
	}

}

// Enum <> String
#define NxEnumType_GetString(enumT, enumV)		Nx_UtilityEnum::EnumToString<enumT::Type>(enumV)			// namespace::type (Enum)
#define NxEnumType_GetDisplayName(enumT, enumV)	Nx_UtilityEnum::EnumToDisplayName<enumT::Type>(enumV)		// namespace::type (Enum)
#define NxEnumType_GetEnum(enumT, stringV)		Nx_UtilityEnum::StringToEnum<enumT::Type>(stringV)		

#define NxEnum_GetString(enumT, enumV)			Nx_UtilityEnum::EnumToString<enumT>(enumV)		
#define NxEnum_GetDisplayName(enumT, enumV)		Nx_UtilityEnum::EnumToDisplayName<enumT>(enumV)		
#define NxEnum_GetEnum(enumT, stringV)			Nx_UtilityEnum::StringToEnum<enumT>(stringV)		

#define NxEnum_GetAnsiChar(enumT, enumV)		TCHAR_TO_ANSI(*Nx_UtilityEnum::EnumToString<enumT>(enumV))
