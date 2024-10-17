
#include "NxCodeWindowsLibrary.h"
#include "NxCodeWindow.h"
#include "NxCodeWindowsModule.h"
#include "Engine/Engine.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Internationalization/Internationalization.h"
#include "Components/SlateWrapperTypes.h"
#include "Misc/CoreDelegates.h"


#define LOCTEXT_NAMESPACE "NxCodeWindowsLibrary"

UNxCodeWindowsLibrary::UNxCodeWindowsLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UNxCodeWindowsLibrary::GetMultiWindowsManager(UNxCodeWindowsManager*& MultiWindowsManager)
{
	FNxCodeWindowsModule& multiWindowModule = FModuleManager::LoadModuleChecked<FNxCodeWindowsModule>("NxCodeWindows");

	multiWindowModule.GetMultiWindowsManager(MultiWindowsManager);
}

void UNxCodeWindowsLibrary::AddWidgetToWindow(UUserWidget* UserWidget, UNxCodeWindow* Window, int32 ZOrder)
{
	if (!UserWidget || !Window || !Window->GameViewportClient)
	{
		return;
	}

	{
		// 사용자가 이 함수를 두 번 호출하지 않도록 변수를 생성하고 초기화합니다.
		TSharedRef<SWidget> userSlateWidget = UserWidget->TakeWidget();
		UPanelWidget* parentPanel = UserWidget->GetParent();
		if (parentPanel != nullptr || userSlateWidget->GetParentWidget().IsValid())
		{
			FMessageLog("PIE").Error(FText::Format(LOCTEXT("WidgetAlreadyHasParent", "위젯 '{0}'에 이미 상위 위젯이 있어서, 뷰포트에 추가할 수 없습니다!"), 	FText::FromString(UserWidget->GetClass()->GetName())));
			return;
		}

		// 위젯을 뷰포트에 다시 추가하려고 시도합니다.
		TSharedRef<SConstraintCanvas> fullScreenCanvas = SNew(SConstraintCanvas);

		fullScreenCanvas->AddSlot()
			//.Offset(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateUObject(UserWidget, &UUserWidget::GetFullScreenOffset)))
			.Anchors(TAttribute<FAnchors>::Create(TAttribute<FAnchors>::FGetter::CreateUObject(UserWidget, &UUserWidget::GetAnchorsInViewport)))
			.Alignment(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateUObject(UserWidget, &UUserWidget::GetAlignmentInViewport)))
			[
				userSlateWidget
			];

		// 모바일 빌드의 가상 조이스틱과 같은 내장 컨트롤 아래에 표시되지 않도록 뷰포트에 추가할 때 zorder 에 10을 추가
		Window->GameViewportClient->AddViewportWidgetContent(fullScreenCanvas, ZOrder + Window->UserWidgetsInViewport.Num() + 10);
		Window->UserWidgetsInViewport.Add(UserWidget);

		// 이미 연결한 경우에 대비하여 처리기를 제거.
		FWorldDelegates::LevelRemovedFromWorld.RemoveAll(Window);

		// persistent 레벨이 언로드되면 뷰포트에 추가된 위젯이 자동으로 제거.
		FWorldDelegates::LevelRemovedFromWorld.AddUObject(Window, &UNxCodeWindow::OnLevelRemovedFromWorldAndRemoveWidgetsInViewport);
	}
}

void UNxCodeWindowsLibrary::RemoveWidgetFromWindow(UUserWidget* UserWidget)
{
	if (UserWidget == nullptr)
		return;

	TSharedRef<SWidget> userSlateWidget = UserWidget->TakeWidget();
	TSharedPtr<SWidget> widgetHost = userSlateWidget->GetParentWidget();

	UNxCodeWindowsManager* multiWindowsManager;
	UNxCodeWindowsLibrary::GetMultiWindowsManager(multiWindowsManager);

	UNxCodeWindow* parentWindow = nullptr;

	if (multiWindowsManager)
	{
		for (auto subWindow : multiWindowsManager->SubWindows)
		{
			if (subWindow == nullptr)
				continue;

			for (auto userWidgetInViewport : subWindow->UserWidgetsInViewport)
			{
				if (userWidgetInViewport && UserWidget == userWidgetInViewport)
				{
					parentWindow = subWindow;
					break;
				}
			}

			if (parentWindow)
			{
				break;
			}
		}

		if (!parentWindow)
		{
			UNxCodeWindow* mainWindow;
			multiWindowsManager->GetMainWindow(mainWindow);
			if (mainWindow)
			{
				for (auto UserWidgetInViewport : mainWindow->UserWidgetsInViewport)
				{
					if (UserWidgetInViewport && UserWidget == UserWidgetInViewport)
					{
						parentWindow = mainWindow;
						break;
					}
				}
			}
		}
	}

	if (parentWindow == nullptr)
	{
		UserWidget->RemoveFromParent();
		return;
	}

	if (!UserWidget->HasAnyFlags(RF_BeginDestroyed))
	{
		if (widgetHost.IsValid())
		{
			// game World 일 경우, 뷰포트에 위젯을 추가
			UWorld* World = UserWidget->GetWorld();

			if (World && World->IsGameWorld())
			{
				if (UGameViewportClient* viewportClient = parentWindow->GameViewportClient)
				{
					TSharedRef<SWidget> widgetHostRef = widgetHost.ToSharedRef();

					viewportClient->RemoveViewportWidgetContent(widgetHostRef);

					if (ULocalPlayer* localPlayer = UserWidget->GetOwningLocalPlayer())
					{
						viewportClient->RemoveViewportWidgetForPlayer(localPlayer, widgetHostRef);
					}
				}
			}
		}
		else
		{
			UserWidget->RemoveFromParent();
		}
	}

	parentWindow->UserWidgetsInViewport.Remove(UserWidget);
}

#undef LOCTEXT_NAMESPACE