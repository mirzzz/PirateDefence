// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeUILibrary.h"
#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "Engine/AssetManager.h"

#include "NxCodeUISubsystem.h" 

int32 UNxCodeUILibrary::InputSuspensions = 0;

ECommonInputType UNxCodeUILibrary::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType();
		}
	}

	return ECommonInputType::Count;
}

bool UNxCodeUILibrary::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
		}
	}
	return false;
}

bool UNxCodeUILibrary::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		}
	}
	return false;
}

//UCommonActivatableWidget* UVxUIBPLibrary::PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
//{
//	////if (!WidgetClass)
//	////{
//	////	//UE_LOG ERROR
//	////	return nullptr;
//	////}
//
//	//if (UVxManagerUI* managerUI = LocalPlayer->GetGameInstance()->GetSubsystem<UVxManagerUI>())
//	//{
//	//	if (UVxUI_Policy* policyUI = managerUI->GetCurrentPolicyUI())
//	//	{
//	//		if (UVxUI_Frame* frameUI = policyUI->GetFrameLayout(CastChecked<UVxLocalPlayer>(LocalPlayer)))
//	//		{
//	//			return frameUI->PushWidgetToLayerStack(LayerName, WidgetClass);
//	//		}
//	//	}
//	//}
//
//	return nullptr;
//}
//
//void UVxUIBPLibrary::PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
//{
//	//if (UVxManagerUI* managerUI = LocalPlayer->GetGameInstance()->GetSubsystem<UVxManagerUI>())
//	//{
//	//	if (UVxUI_Policy* policyUI = managerUI->GetCurrentPolicyUI())
//	//	{
//	//		if (UVxUI_Frame* frameUI = policyUI->GetFrameLayout(CastChecked<UVxLocalPlayer>(LocalPlayer)))
//	//		{
//	//			const bool bSuspendInputUntilComplete = true;
//
//	//			frameUI->PushWidgetToLayerStackAsync(LayerName, bSuspendInputUntilComplete, WidgetClass);
//	//		}
//	//	}
//	//}
//}
//
//void UVxUIBPLibrary::PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget)
//{
//	if (const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
//	{
//		//if (const UVxManagerUI* managerUI = LocalPlayer->GetGameInstance()->GetSubsystem<UVxManagerUI>())
//		//{
//		//	if (const UVxUI_Policy* policyUI = managerUI->GetCurrentPolicyUI())
//		//	{
//		//		if (UVxUI_Frame* frameUI = policyUI->GetFrameLayout(CastChecked<UVxLocalPlayer>(LocalPlayer)))
//		//		{
//		//			frameUI->FindAndRemoveWidgetFromLayer(ActivatableWidget);
//		//		}
//		//	}
//		//}
//	}
//}

void UNxCodeUILibrary::OpenAsync(TSubclassOf<UCommonActivatableWidget> ActivatableWidget, uint8 FramePos, uint8 eLayer)
{
	UNxCodeUISubsystem::Get()->OpenAsync(ActivatableWidget, FramePos, (E_StackLayer)eLayer);
}

ULocalPlayer* UNxCodeUILibrary::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		return Cast<ULocalPlayer>(PlayerController->Player);
	}

	return nullptr;
}

FName UNxCodeUILibrary::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UNxCodeUILibrary::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UNxCodeUILibrary::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UNxCodeUILibrary::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}

bool UNxCodeUILibrary::LineIntersection2D(FVector2D& A, FVector2D& A2, FVector2D& B, FVector2D& B2, FVector2D& Out)
{
	FVector Out3 = FVector::ZeroVector;
	bool result = FMath::SegmentIntersection2D(FVector(A.X, A.Y, 0), FVector(A2.X, A2.Y, 0), FVector(B.X, B.Y, 0), FVector(B2.X, B2.Y, 0), Out3);
	Out = FVector2D(Out3.X, Out3.Y);
	return result;
}


void UNxCodeUILibrary::BoundWindowClipLine(TArray<FVector2D>& Lines, FVector2D& WindowSize, TArray<FVector2D>& OutLine)
{
	FVector2D	 windowHalf =  WindowSize * 0.5f;
	TArray<bool> outofbound;
	for (int i = 0; i < Lines.Num(); ++i)
	{
		// 범위 벗어남
		if (windowHalf.X < FMath::Abs(Lines[i].X) || windowHalf.Y < FMath::Abs(Lines[i].Y))
		{
			outofbound.Add(true);
		}
		else
		{
			outofbound.Add(false);
		}
	}

	// 벗어난 위치를 라인 교차로 제조정 한다.
	for (int i = 0; i < Lines.Num()-1; ++i)
	{
		// 교차가 발생할수 있는 구간
		if (outofbound[i] != outofbound[i+1])
		{
			FVector2D TL = FVector2D(-windowHalf.X, -windowHalf.Y);
			FVector2D TR = FVector2D(windowHalf.X, -windowHalf.Y);
			FVector2D BL = FVector2D(-windowHalf.X, windowHalf.Y);
			FVector2D BR = FVector2D(windowHalf.X, windowHalf.Y);

			FVector2D Intersect;
			if (UNxCodeUILibrary::LineIntersection2D(TL, TR, Lines[i], Lines[i + 1], Intersect))
			{
				if (outofbound[i] == false)		{ OutLine.Add(Lines[i]);  OutLine.Add(Intersect); }
				if (outofbound[i + 1] == false) { OutLine.Add(Intersect); OutLine.Add(Lines[i + 1]); }
			}
			else if(UNxCodeUILibrary::LineIntersection2D(BL, BR, Lines[i], Lines[i + 1], Intersect))
			{
				if (outofbound[i] == false)		{ OutLine.Add(Lines[i]);  OutLine.Add(Intersect); }
				if (outofbound[i + 1] == false) { OutLine.Add(Intersect); OutLine.Add(Lines[i + 1]); }
			}
			else if (UNxCodeUILibrary::LineIntersection2D(TL, BL, Lines[i], Lines[i + 1], Intersect))
			{
				if (outofbound[i] == false)		{ OutLine.Add(Lines[i]);  OutLine.Add(Intersect); }
				if (outofbound[i + 1] == false) { OutLine.Add(Intersect); OutLine.Add(Lines[i + 1]); }
			}
			else if (UNxCodeUILibrary::LineIntersection2D(TR, BR, Lines[i], Lines[i + 1], Intersect))
			{
				if (outofbound[i] == false)		{ OutLine.Add(Lines[i]);  OutLine.Add(Intersect); }
				if (outofbound[i + 1] == false) { OutLine.Add(Intersect); OutLine.Add(Lines[i + 1]); }
			}

		}
		else if(outofbound[i] == false && outofbound[i+1] == false)
		{
			OutLine.Add(Lines[i]);
			OutLine.Add(Lines[i+1]);
		}
	}
}
