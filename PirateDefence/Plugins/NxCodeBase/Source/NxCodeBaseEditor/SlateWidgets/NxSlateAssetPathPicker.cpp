// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxSlateAssetPathPicker.h"

#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "NxSlateAssetPathPicker"

void SNxSlateAssetPathPicker::Construct(const FArguments& InArgs)
{
	AssetPath = FText::FromString(FPackageName::GetLongPackagePath(InArgs._DefaultAssetPath.ToString()));
	AssetName = FText::FromString(FPackageName::GetLongPackageAssetName(InArgs._DefaultAssetPath.ToString()));

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath     = AssetPath.ToString();	
	PathPickerConfig.OnPathSelected  = FOnPathSelected::CreateSP(this, &SNxSlateAssetPathPicker::OnPathChange);
	PathPickerConfig.bAddDefaultPath = true;
	PathPickerConfig.bShowFavorites = false;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	SWindow::Construct(
		SWindow::FArguments()
		.Title(InArgs._Title)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		//.SizingRule( ESizingRule::Autosized )
		.ClientSize(FVector2D(450, 450))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot() // Add user input block
			.Padding(2, 2, 2, 4)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1)
					.Padding(3)
					[
						ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig)
					]
				]
			]

			// https://superyateam.com/2021/07/19/ue4-slate-widget-example/
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			[
				SAssignNew(AssetNameEditorBox, SEditableTextBox)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.HintText(LOCTEXT("AssetNameHint", "Fill Asset name"))
				.Text(AssetName)
				.OnTextChanged(this, &SNxSlateAssetPathPicker::OnNameChange)
				.OnTextCommitted(this, &SNxSlateAssetPathPicker::OnNameCommitted)
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
				.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
				.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
				+SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("OK", "OK"))
					.HAlign(HAlign_Center)
					.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SNxSlateAssetPathPicker::OnButtonClick, EAppReturnType::Ok)
				]
				+SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("Cancel", "Cancel"))
					.HAlign(HAlign_Center)
					.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SNxSlateAssetPathPicker::OnButtonClick, EAppReturnType::Cancel)
				]
			]
		]);
}


void SNxSlateAssetPathPicker::OnPathChange(const FString& NewPath)
{
	AssetPath = FText::FromString(NewPath);
}

void SNxSlateAssetPathPicker::OnNameChange(const FText& NewName)
{
	AssetName = NewName;
}

void SNxSlateAssetPathPicker::OnNameCommitted(const FText& NewName, ETextCommit::Type CommitInfo)
{
	if (CommitInfo == ETextCommit::OnEnter)
	{
		AssetName = NewName;
	}
}

FReply SNxSlateAssetPathPicker::OnButtonClick(EAppReturnType::Type ButtonID)
{
	UserResponse = ButtonID;

	if (ButtonID == EAppReturnType::Cancel || ValidatePackage())
	{
		// Only close the window if canceling or if the asset name is valid
		RequestDestroyWindow();
	}
	else
	{
		// reset the user response in case the window is closed using 'x'.
		UserResponse = EAppReturnType::Cancel;
	}

	return FReply::Handled();
}

/** Ensures supplied package name information is valid */
bool SNxSlateAssetPathPicker::ValidatePackage()
{
	FText Reason;
	if (!FPackageName::IsValidLongPackageName(GetFullAssetPath().ToString(), false, &Reason)
		|| !FName(*AssetName.ToString()).IsValidObjectName(Reason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return false;
	}

	if (FPackageName::DoesPackageExist(GetFullAssetPath().ToString()) ||
		FindObject<UObject>(NULL, *(AssetPath.ToString() + "/" + AssetName.ToString() + "." + AssetName.ToString())) != NULL)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("AssetAlreadyExists", "Asset {0} already exists."), GetFullAssetPath()));
		return false;
	}

	return true;
}

EAppReturnType::Type SNxSlateAssetPathPicker::ShowModal()
{
	GEditor->EditorAddModalWindow(SharedThis(this));
	return UserResponse;
}

const FText& SNxSlateAssetPathPicker::GetAssetPath()
{
	return AssetPath;
}

const FText& SNxSlateAssetPathPicker::GetAssetName()
{
	return AssetName;
}

FText SNxSlateAssetPathPicker::GetFullAssetPath()
{
	return FText::FromString(AssetPath.ToString() + "/" + AssetName.ToString());
}

#undef LOCTEXT_NAMESPACE