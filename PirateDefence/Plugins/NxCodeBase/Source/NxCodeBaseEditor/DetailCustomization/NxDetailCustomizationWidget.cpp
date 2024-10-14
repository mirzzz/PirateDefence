// Fill out your copyright notice in the Description page of Project Settings.


#include "DetailCustomization/NxDetailCustomizationWidget.h"
#include "NxCodeBaseEditorModule.h"

// Ŀ���͸���¡ ��� Ŭ���� ����, ��� Ŭ������ UClass�� �Ļ����� Ŭ����
#include "DetailCustomization/NxDetailCustomizationActor.h"

// PropertyEditor
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"

// Slate
#include "Fonts/SlateFontInfo.h"
#include "Widgets/Text/STextBlock.h"		
#include "Widgets/Input/SButton.h"
#include "Framework/Commands/UIAction.h"

#define LOCTEXT_NAMESPACE "NxDetailCustomizationWidget"


TSharedRef<IDetailCustomization> FNxDetailCustomizationWidget::MakeInstance()
{
	return MakeShareable(new FNxDetailCustomizationWidget);
}

void FNxDetailCustomizationWidget::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Ŀ���͸���¡ ��� Ŭ���� �̸� Ȯ��
	UE_LOG(LogCodeBaseEditor, Warning, TEXT("CustomizeDetails : %s"), *DetailBuilder.GetBaseClass()->GetName());

	// ECategoryPriority::Variable, Transform, Important, TypeSpecific
	//	ī�װ� �켱������ ���� UnrealEditor �� UI�� ���� �� ��, ������ ���� �Ҽ� �ִ�.
	
	// Ŀ���͸���¡ ��� ���� �ν��Ͻ� ����
	DetailBuilder.GetObjectsBeingCustomized(OwnerObjects);

	// ī�װ� �߰�
	{
		// ī�װ� ����
		IDetailCategoryBuilder& builderTextCategory = DetailBuilder.EditCategory("NxAddCategory", FText::FromString(TEXT("Test.AddCategory")), ECategoryPriority::Variable);
		// ���� ������
		TSharedRef<IPropertyHandle> bindStringProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomString));
		// ������ ���ο� ī�װ��� �̵� ��Ŵ
		builderTextCategory.AddProperty(bindStringProperty);
	}
		
	// ���� �߰�
	{
		// Text �߰�.
		IDetailCategoryBuilder& builderTextRow = DetailBuilder.EditCategory("NxAddText", FText::FromString(TEXT("Test.AddText")), ECategoryPriority::Variable);
		builderTextRow.AddCustomRow(FText::FromString("Simple Text Row"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Simple Text"))
				.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
				.ColorAndOpacity(FSlateColor(FColor::Green))
				.ShadowOffset(FVector2D(-1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor::Red)
				.Margin(FMargin(0.0f, 5.0f, 0.0f, 5.0f))
			];

		// Name-Value �߰�.
		IDetailCategoryBuilder& builderNameValueRow = DetailBuilder.EditCategory("NxAddNameValue", FText::FromString(TEXT("Test.AddNameValue")), ECategoryPriority::Variable);
		builderNameValueRow.AddCustomRow(FText::FromString("Simple NameValue Row"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("User Name"))
			]
			.ValueContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("User Value"))
			];
	}

	// Actor ��ȣ�ۿ� - ����� ���
	{
		IDetailCategoryBuilder& builderVisibility = DetailBuilder.EditCategory("NxVisibility", FText::FromString(TEXT("Test.Visiblity")), ECategoryPriority::Variable);
		TSharedRef<IPropertyHandle> bindBoolProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomBool));

		// ī�װ��� bool ���� �̵�
		builderVisibility.AddProperty(bindBoolProperty);
				
		if (OwnerObjects.Num() != 0)
		{
			for (auto& object : OwnerObjects)
			{
				TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast<ANxDetailCustomizationActor>(object.Get());
			
				// customActor�� CustomBool ���� true�̸�, EVisibility::Visible ��ȯ�ϴ� Functor ����
				auto OnGetVisibility = [customActor]()
				{
					return customActor.IsValid() && customActor->CustomBool ? EVisibility::Visible : EVisibility::Hidden;
				};

				// Visible/Hidden ��ȯ ���ٽ� ����
				auto VisibilityAttr = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnGetVisibility));

				// bool ���¿� ���� Text �߰�
				builderVisibility.AddCustomRow(FText::FromString("Simple Visible Text Row"))
					.Visibility(VisibilityAttr)		// Visibility ����
					.WholeRowContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Bool Value is Changing!"))
						.Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
						.ColorAndOpacity(FSlateColor(FColor::Green))
						.Margin(FMargin(100.0f, 5.0f, 0.0f, 5.0f))
					];
			}
		}		
	}

	// ��ư �߰�
	{
		IDetailCategoryBuilder& builderButton = DetailBuilder.EditCategory("NxButton", FText::FromString(TEXT("Test.Button")), ECategoryPriority::Variable);
		
		// Ŀ���͸���¡ ��� ���� �ν��Ͻ� ����
		if (OwnerObjects.Num() != 0)
		{
			for (auto& object : OwnerObjects)
			{
				TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast< ANxDetailCustomizationActor>(object.Get());

				// ��ư Hover
				auto OnButtonHover = [customActor]()
				{
					UE_LOG(LogCodeBaseEditor, Warning, TEXT("Button Hover! %s"), *customActor->GetClass()->GetName());
				};

				// ��ư Click
				auto OnButtonClick = [customActor]()
				{
					UE_LOG(LogCodeBaseEditor, Warning, TEXT("Button Click! %s"), *customActor->GetClass()->GetName());

					return FReply::Handled();
				};

				builderButton.AddCustomRow(FText::FromString("Simple Button"))
					.WholeRowContent()
					[
						SNew(SButton)
						.Text(FText::FromString("DetailCustomization Button"))						
						.OnClicked_Lambda(OnButtonClick)
						.OnHovered_Lambda(OnButtonHover)
					];

			}
		}
	}

	// �޺� �߰�
	{
		IDetailCategoryBuilder& builderCombo = DetailBuilder.EditCategory("NxCombo", FText::FromString(TEXT("Test.Combo")), ECategoryPriority::Variable);
		TSharedRef<IPropertyHandle> bindComboProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomComboList));
				
		//// Property �� ����Ǹ� UI�� ���� ��Ų��.
		//const FSimpleDelegate OnComboValueChanged = FSimpleDelegate::CreateLambda([&DetailBuilder]()
		//	{
		//		DetailBuilder.ForceRefreshDetails();
		//	});

		//// ANxDetailCustomizationActor->CustomComboList Property�� ����� �̺�Ʈ �����Ŵ
		//const TSharedRef<IPropertyHandle> CommboPropertyHandle = DetailBuilder.GetProperty(TEXT("CustomComboList") );
		//if (CommboPropertyHandle->IsValidHandle())
		//{
		//	CommboPropertyHandle->SetOnPropertyValueChanged(OnComboValueChanged);
		//}

		// Ŀ���͸���¡ ��� ���� �ν��Ͻ� ����
		if (OwnerObjects.Num() != 0)
		{
			for (auto& object : OwnerObjects)
			{
				TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast< ANxDetailCustomizationActor>(object.Get());
				if (customActor.Get() && customActor.IsValid())
				{
					if( customActor->CustomComboChoose.IsEmpty() && customActor->CustomComboList.Num() != 0 )
						customActor->CustomComboChoose = customActor->CustomComboList[0];

					Comobo_Selected = customActor->CustomComboChoose;
				}

				// customActor�� CustomCombos ����Ʈ�� ������ true�̸�, EVisibility::Visible ��ȯ�ϴ� Functor ����
				auto OnGetComoboVisible = [customActor]()
				{
					return customActor.IsValid() && customActor->CustomComboList.Num() != 0 ? EVisibility::Visible : EVisibility::Hidden;
				};

				// Visible/Hidden ��ȯ ���ٽ� ����
				auto ComboVisibilityAttr = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnGetComoboVisible));

				// bool ���¿� ���� Text �߰�
				builderCombo.AddCustomRow(FText::FromString("Simple Combo"))
					.Visibility(ComboVisibilityAttr)		// Visibility ����
					[
						SNew(SComboButton)
						.OnGetMenuContent(this, &FNxDetailCustomizationWidget::OnComboMakeMenu)	// �޴� ����
						.ButtonContent()														// ���� ����
						[
							SNew(STextBlock).
							Text_Lambda([this]()
							{
								return FText::FromString(Comobo_Selected);
							})
						]
					];
			}
		}
	}
}

TSharedRef<SWidget> FNxDetailCustomizationWidget::OnComboMakeMenu()
{
	for (auto& object : OwnerObjects)
	{
		TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast<ANxDetailCustomizationActor>(object.Get());
		if (customActor.Get() && customActor.IsValid())
		{
			TArray<FString> comboList = customActor->CustomComboList;

			if (comboList.IsEmpty() == false)
			{
				FMenuBuilder menuBuilder(true, nullptr);

				for (int i = 0; i < comboList.Num(); i++)
				{
					FString     comboName = comboList[i];
					FUIAction   comboAction = FUIAction(FExecuteAction::CreateSP(this, &FNxDetailCustomizationWidget::OnComboChange, comboName));
					menuBuilder.AddMenuEntry(FText::FromString(comboName), FText(), FSlateIcon(), comboAction);
				}

				return menuBuilder.MakeWidget();
			}
		}
	}

	return SNullWidget::NullWidget;
}

void FNxDetailCustomizationWidget::OnComboChange(FString SelectCombo)
{
	Comobo_Selected = SelectCombo;

	if (OwnerObjects.Num() != 0)
	{
		for (auto& object : OwnerObjects)
		{
			TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast<ANxDetailCustomizationActor>(object.Get());
			if (customActor.IsValid())
			{
				customActor->CustomComboChoose = SelectCombo;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE //"VxDetailCustomizationSample"