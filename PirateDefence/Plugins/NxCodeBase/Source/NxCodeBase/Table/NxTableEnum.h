#pragma once

#include "CoreMinimal.h"
#include "NxTableEnum.generated.h"

// TableSetType
//--------------------------------------------------------------------------
UENUM(BlueprintType)
enum class E_TableCategory : uint8
{
	None			UMETA(DisplayName = "None"), 
	DataTable		UMETA(DisplayName = "UnrealDataTable"),
	DataXml			UMETA(DisplayName = "Xml File"),
	DataJson		UMETA(DisplayName = "Json File"),
	StringTable		UMETA(DisplayName = "UnrealStringTable"),
	Max
};

// UDataTable 종류
//--------------------------------------------------------------------------
UENUM(BlueprintType)
enum class E_DataTable : uint8
{
	None,
	Template,			// 테스트 템플릿.
	Map,				// 맵	
	Sound,				// sound
	UI,					// ui
	FX,					// Effect
	Max,
};

// Xml 파일 종류
//--------------------------------------------------------------------------
UENUM(BlueprintType)
enum class E_XmlData : uint8
{
	None,
	ConfigNet,
	ConfigApp,
	ConfigGame,
	Max,
};


// Json 파일 종류
//--------------------------------------------------------------------------
UENUM(BlueprintType)
enum class E_JsonData : uint8
{
	None,
	CourseSpline,
	Max,
};

// StringTable 종류
//--------------------------------------------------------------------------
UENUM(BlueprintType)
enum class E_StringTable : uint8
{
	None,
	Error,
	Enum,
	Common,
	UI,
	Net,
	Max,
};