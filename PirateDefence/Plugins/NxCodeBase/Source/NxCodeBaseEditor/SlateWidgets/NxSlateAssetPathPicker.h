
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWindow.h"


#define LOCTEXT_NAMESPACE "NxSlateAssetPathPicker"


/*
* Asset 경로를 얻기 위한 대화창 생성 - Editor

	// 디렉토리를 설정하여, 출력할 경로를 생성한다.
	FString NewNameSuggestion	  = FString(TEXT("ExportName"));
	FString PackageNameSuggestion = FString(TEXT("/Game/")) + NewNameSuggestion;

	TSharedPtr<SNxSlateAssetPathPicker> PickAssetPathWidget =
		SNew(SNxSlateAssetPathPicker)
		.Title(LOCTEXT("ExportFile", "Export, Path & Filename"))
		.DefaultAssetPath(FText::FromString(PackageNameSuggestion));

	if (PickAssetPathWidget->ShowModal() == EAppReturnType::Ok)
	{
		FString PackageName = PickAssetPathWidget->GetFullAssetPath().ToString();
	}
*/ 
class SNxSlateAssetPathPicker : public SWindow
{
	TSharedPtr<SEditableTextBox> AssetNameEditorBox;

public:
	SLATE_BEGIN_ARGS(SNxSlateAssetPathPicker) {}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, DefaultAssetPath)
	SLATE_END_ARGS()

	SNxSlateAssetPathPicker()
		: UserResponse(EAppReturnType::Cancel)
	{
	}

	void Construct(const FArguments& InArgs);

	/** Displays the dialog in a blocking fashion */
	EAppReturnType::Type ShowModal();

	/** Gets the resulting asset path */
	const FText& GetAssetPath();

	/** Gets the resulting asset name */
	const FText& GetAssetName();

	/** Gets the resulting full asset path (path+'/'+name) */
	FText GetFullAssetPath();

protected:
	void OnPathChange(const FString& NewPath);

	void OnNameChange(const FText& NewName);
	void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitInfo);

	FReply OnButtonClick(EAppReturnType::Type ButtonID);
	
	bool ValidatePackage();

	EAppReturnType::Type UserResponse;
	FText AssetPath;
	FText AssetName;
};

#undef LOCTEXT_NAMESPACE