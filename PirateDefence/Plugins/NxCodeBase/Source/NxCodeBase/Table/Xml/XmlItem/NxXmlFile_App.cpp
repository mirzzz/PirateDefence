// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Xml/XmlItem/NxXmlFile_App.h"
#include "Misc/Paths.h"
#include "NxBaseLog.h"

//------------------------------------------------------------------------------------ Tag
namespace Nx_XmlTAG
{
	const FString Default_Filename		= TEXT("Content/NUFS/Table/ConfigApp.xml");

	const FString Xml_EnvApp_Version	= TEXT("Version");
	const FString Xml_EnvApp_Auth		= TEXT("Auth");
	const FString Xml_EnvApp_AuthKey	= TEXT("AuthKey");
}



//------------------------------------------------------------------------------------ Data


//------------------------------------------------------------------------------------ Class
UVxXmlFile_App::UVxXmlFile_App()
{
}

bool UVxXmlFile_App::Parsing(FString& InFilename)
{
	if (InFilename.IsEmpty())
	{
		FString xmlfullpath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + Nx_XmlTAG::Default_Filename;
		InFilename = xmlfullpath;
	}

	if (OpenXmlFile(InFilename) == false)
		return false;

	// Parsing Test...
	const FXmlNode* RootNode = GetRootNode();

	NxCheckReturn(RootNode != nullptr, false);

	TArray<FString> AuthList;
	
	AuthList.Add(Nx_XmlTAG::Xml_EnvApp_Auth);

	NxPrintLog(LogBase, TEXT("Xml[%s] Tag[%s] ChildrenNodes[%d]"), *InFilename, *RootNode->GetTag(), RootNode->GetChildrenNodes().Num());
		
	XmlData.Version = GetContent<int32>(RootNode, Nx_XmlTAG::Xml_EnvApp_Version);
	
	for (auto& authInfo : AuthList)
	{
		TArray<FXmlNode*> findAuthNode = GetNodes(authInfo);
		for (auto& node : findAuthNode) 
		{
			FXmlData_AuthInfo  auth;
			auth.Key = GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvApp_AuthKey);
			
			XmlData.AuthInfo = auth;
		}
	}
	
	Close();

	return true;
}

// Binary 저장 (지원 안함)
bool UVxXmlFile_App::SaveBinary(FString filename)
{
	return false;
}

// Binary 읽기 (지원 안함)
bool UVxXmlFile_App::LoadBinary(FString filename)
{
	return false;
}

