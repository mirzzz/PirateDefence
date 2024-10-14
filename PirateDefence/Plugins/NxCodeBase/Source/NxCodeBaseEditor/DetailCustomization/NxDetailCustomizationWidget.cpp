// Fill out your copyright notice in the Description page of Project Settings.


#include "DetailCustomization/NxDetailCustomizationWidget.h"
#include "NxCodeBaseEditorModule.h"

// 커스터마이징 대상 클래스 포함, 대상 클래스는 UClass를 파생받은 클래스
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
	// 커스터마이징 대상 클래스 이름 확인
	UE_LOG(LogCodeBaseEditor, Warning, TEXT("CustomizeDetails : %s"), *DetailBuilder.GetBaseClass()->GetName());

	// ECategoryPriority::Variable, Transform, Important, TypeSpecific
	//	카테고리 우선순위에 따라 UnrealEditor 에 UI로 노출 될 때, 순서를 조정 할수 있다.
	
	// 커스터마이징 대상 액터 인스턴스 참조
	DetailBuilder.GetObjectsBeingCustomized(OwnerObjects);

	// 카테고리 추가
	{
		// 카테고리 생성
		IDetailCategoryBuilder& builderTextCategory = DetailBuilder.EditCategory("NxAddCategory", FText::FromString(TEXT("Test.AddCategory")), ECategoryPriority::Variable);
		// 변수 가져옴
		TSharedRef<IPropertyHandle> bindStringProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomString));
		// 변수를 새로운 카테고리로 이동 시킴
		builderTextCategory.AddProperty(bindStringProperty);
	}
		
	// 위젯 추가
	{
		// Text 추가.
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

		// Name-Value 추가.
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

	// Actor 상호작용 - 숨기기 기능
	{
		IDetailCategoryBuilder& builderVisibility = DetailBuilder.EditCategory("NxVisibility", FText::FromString(TEXT("Test.Visiblity")), ECategoryPriority::Variable);
		TSharedRef<IPropertyHandle> bindBoolProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomBool));

		// 카테고리로 bool 변수 이동
		builderVisibility.AddProperty(bindBoolProperty);
				
		if (OwnerObjects.Num() != 0)
		{
			for (auto& object : OwnerObjects)
			{
				TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast<ANxDetailCustomizationActor>(object.Get());
			
				// customActor의 CustomBool 값이 true이면, EVisibility::Visible 반환하는 Functor 생성
				auto OnGetVisibility = [customActor]()
				{
					return customActor.IsValid() && customActor->CustomBool ? EVisibility::Visible : EVisibility::Hidden;
				};

				// Visible/Hidden 반환 람다식 구성
				auto VisibilityAttr = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnGetVisibility));

				// bool 상태에 따른 Text 추가
				builderVisibility.AddCustomRow(FText::FromString("Simple Visible Text Row"))
					.Visibility(VisibilityAttr)		// Visibility 설정
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

	// 버튼 추가
	{
		IDetailCategoryBuilder& builderButton = DetailBuilder.EditCategory("NxButton", FText::FromString(TEXT("Test.Button")), ECategoryPriority::Variable);
		
		// 커스터마이징 대상 액터 인스턴스 참조
		if (OwnerObjects.Num() != 0)
		{
			for (auto& object : OwnerObjects)
			{
				TWeakObjectPtr<ANxDetailCustomizationActor> customActor = Cast< ANxDetailCustomizationActor>(object.Get());

				// 버튼 Hover
				auto OnButtonHover = [customActor]()
				{
					UE_LOG(LogCodeBaseEditor, Warning, TEXT("Button Hover! %s"), *customActor->GetClass()->GetName());
				};

				// 버튼 Click
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

	// 콤보 추가
	{
		IDetailCategoryBuilder& builderCombo = DetailBuilder.EditCategory("NxCombo", FText::FromString(TEXT("Test.Combo")), ECategoryPriority::Variable);
		TSharedRef<IPropertyHandle> bindComboProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ANxDetailCustomizationActor, CustomComboList));
				
		//// Property 가 변경되면 UI를 갱신 시킨다.
		//const FSimpleDelegate OnComboValueChanged = FSimpleDelegate::CreateLambda([&DetailBuilder]()
		//	{
		//		DetailBuilder.ForceRefreshDetails();
		//	});

		//// ANxDetailCustomizationActor->CustomComboList Property에 변경시 이벤트 적용시킴
		//const TSharedRef<IPropertyHandle> CommboPropertyHandle = DetailBuilder.GetProperty(TEXT("CustomComboList") );
		//if (CommboPropertyHandle->IsValidHandle())
		//{
		//	CommboPropertyHandle->SetOnPropertyValueChanged(OnComboValueChanged);
		//}

		// 커스터마이징 대상 액터 인스턴스 참조
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

				// customActor의 CustomCombos 리스트가 있으면 true이면, EVisibility::Visible 반환하는 Functor 생성
				auto OnGetComoboVisible = [customActor]()
				{
					return customActor.IsValid() && customActor->CustomComboList.Num() != 0 ? EVisibility::Visible : EVisibility::Hidden;
				};

				// Visible/Hidden 반환 람다식 구성
				auto ComboVisibilityAttr = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(OnGetComoboVisible));

				// bool 상태에 따른 Text 추가
				builderCombo.AddCustomRow(FText::FromString("Simple Combo"))
					.Visibility(ComboVisibilityAttr)		// Visibility 설정
					[
						SNew(SComboButton)
						.OnGetMenuContent(this, &FNxDetailCustomizationWidget::OnComboMakeMenu)	// 메뉴 생성
						.ButtonContent()														// 선택 내용
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