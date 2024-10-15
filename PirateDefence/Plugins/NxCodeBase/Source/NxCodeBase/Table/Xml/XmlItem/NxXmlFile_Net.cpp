// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Xml/XmlItem/NxXmlFile_Net.h"
#include "Misc/Paths.h"
#include "NxBaseLog.h"

//------------------------------------------------------------------------------------ Tag
namespace Nx_XmlTAG
{
	const FString Default_Filename		= TEXT("Content/NUFS/Table/ConfigNet.xml");

	const FString Xml_EnvNet_Version	= TEXT("Version");
	const FString Xml_EnvNet_Agent		= TEXT("Agent");
	const FString Xml_EnvNet_Server		= TEXT("Server");
	const FString Xml_EnvAcc_Account	= TEXT("Account");


	// Agent/Server
	const FString Xml_EnvNet_Name		= TEXT("name");
	const FString Xml_EnvNet_IP			= TEXT("ip");
	const FString Xml_EnvNet_Port		= TEXT("port");
	const FString Xml_EnvNet_AutoURL	= TEXT("authUrl");
	const FString Xml_EnvNet_Error		= TEXT("error");

	// Accounts
	const FString Xml_EnvAcc_ID			= TEXT("accID");
	const FString Xml_EnvAcc_Nick		= TEXT("nickName");
}



//------------------------------------------------------------------------------------ Data
void FXmlData_ConnectInfo::Clear()
{
	Kind = FString(TEXT("Kind"));
	Name = FString(TEXT("Default"));
	IP	 = FString(TEXT("Unknown"));
	Port = 0;
	ErrorMsg = FString(TEXT("Unknown"));

	AuthURL.Empty();
}

void FXmlData_ConnectInfo::PrintLog()
{
	NxPrintLog(LogBase, TEXT("Name:%s-%s, ip:%s port:%d"), *Kind, *Name, *IP, Port);
}


//------------------------------------------------------------------------------------ Class
UNxXmlFile_Net::UNxXmlFile_Net()
{
}

bool UNxXmlFile_Net::Parsing(FString& InFilename)
{
	if (InFilename.IsEmpty())
	{
		// filename : Content/NUFS/Table/ConfigNet.xml 
		FString xmlfullpath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + Nx_XmlTAG::Default_Filename;
		InFilename = xmlfullpath;
	}

	if (OpenXmlFile(InFilename) == false)
		return false;

	// Parsing Test...
	const FXmlNode* RootNode = GetRootNode();

	NxCheckReturn(RootNode != nullptr, false);

	TArray<FString> ConnectList;
	
	ConnectList.Add(Nx_XmlTAG::Xml_EnvNet_Agent);		
	ConnectList.Add(Nx_XmlTAG::Xml_EnvNet_Server);

	NxPrintLog(LogBase, TEXT("Xml[%s] Tag[%s] ChildrenNodes[%d]"), *InFilename, *RootNode->GetTag(), RootNode->GetChildrenNodes().Num());
		
	XmlData.Version = GetContent<int32>(RootNode, Nx_XmlTAG::Xml_EnvNet_Version);
	
	for (auto& connectInfo : ConnectList)
	{
		FString connectStr = connectInfo;

		// 서버 리스트 추가
		TArray<FXmlNode*> findConnectNode = GetNodes(connectStr);
		for (auto& node : findConnectNode)
		{
			FXmlData_ConnectInfo  xmlConfig;

			xmlConfig.Kind		= connectStr;
			xmlConfig.Name		= GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvNet_Name);
			xmlConfig.IP		= GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvNet_IP);
			xmlConfig.Port		= GetAttribute<int32>	(node, Nx_XmlTAG::Xml_EnvNet_Port);
			xmlConfig.ErrorMsg	= GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvNet_Error);
			xmlConfig.AuthURL	= GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvNet_AutoURL);
			
			XmlData.Connections.Add(xmlConfig);
		}

		// Connections Load 갯수 출력.
		NxPrintLog(LogBase, TEXT("Xml Connections Load Count > %d"), XmlData.Connections.Num());
	}
	
	TArray<FXmlNode*> findNodes;
	findNodes.Empty();
	findNodes = GetNodes(Nx_XmlTAG::Xml_EnvAcc_Account);
	for (auto& node : findNodes)
	{
		FXmlData_AccountInfo accountData;

		accountData.AccountID		= GetAttribute<int64>	(node, Nx_XmlTAG::Xml_EnvAcc_ID);
		accountData.NickName		= GetAttributeToString	(node, Nx_XmlTAG::Xml_EnvAcc_Nick);

		XmlData.Accounts.Add(accountData.AccountID, accountData);
	}

	Close();

	return true;
}

// Binary 저장 (지원 안함)
bool UNxXmlFile_Net::SaveBinary(FString filename)
{
	return false;
}

// Binary 읽기 (지원 안함)
bool UNxXmlFile_Net::LoadBinary(FString filename)
{
	return false;
}

