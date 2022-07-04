// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomMinesweeper.h"

#include "Framework/Docking/TabManager.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "SlateFwd.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SNumericEntryBox.h" 
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"

#include <iostream>

#define LOCTEXT_NAMESPACE "FCustomMinesweeperModule"

static const FName MinesweeperDockTab("Minesweeper");

void FCustomMinesweeperModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MinesweeperDockTab, FOnSpawnTab::CreateRaw(this, &FCustomMinesweeperModule::CreateMinesweeperWindow))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	TSharedRef<FExtender> ToolBarExtender = MakeShared<FExtender>();
	ToolBarExtender->AddToolBarExtension("Settings", EExtensionHook::After, nullptr,
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
			{
				Builder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([this]()
					{
						FGlobalTabmanager::Get()->InvokeTab(MinesweeperDockTab);
					})), NAME_None, FText::FromString("MINESWEEPER"));
			}));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolBarExtender);
}

void FCustomMinesweeperModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MinesweeperDockTab);
	Grid.Empty();
	if (GridPanel.IsValid())
	{
		GridPanel->ClearChildren();
		GridPanel.Reset();
	}
}

TSharedRef<SDockTab> FCustomMinesweeperModule::CreateMinesweeperWindow(const FSpawnTabArgs& TabArgs)
{
	Init();

	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FillHeight(0.1f).Padding(0.f, 5.f) // Width/Height
			[
				CreateSizeSlot()
			]
			+SVerticalBox::Slot().FillHeight(0.1f).Padding(0.f, 5.f) // N Mines
			[
				CreateMinesNumSlot()
			]
			+SVerticalBox::Slot().FillHeight(0.2f).Padding(0.f, 5.f).HAlign(EHorizontalAlignment::HAlign_Left) // Button
			[
				CreateButton(FText::FromString("Create Grid"), &TextStyle, &FCustomMinesweeperModule::CreateGrid)
			]
			+SVerticalBox::Slot().Padding(0.f, 5.f) // UI Grid
			[
				SAssignNew(GridPanel, SGridPanel)
			]
		];	
}

void FCustomMinesweeperModule::Init()
{
	Width = MIN_WIDTH;
	Height = MIN_HEIGHT;
	MinesNum = MIN_MINES_NUM;
	Left = 0;
	TextStyle.Font = FSlateFontInfo((UObject*)GEngine->GetLargeFont(), FontSize);
	TextStyle.ColorAndOpacity = TextColor;
}

TSharedRef<SWidget> FCustomMinesweeperModule::CreateSizeSlot()
{
	auto OnWidthCommit = [this](int32 InValue, ETextCommit::Type CommitType) -> void { Width = InValue; };
	auto OnwidthUpdate = [this]() -> int32 { return Width; };
	
	auto OnHeightCommit = [this](int32 InValue, ETextCommit::Type CommitType) -> void { Height = InValue; };
	auto OnHeightUpdate = [this]() -> int32 { return Height; };
	
	TSharedRef<SHorizontalBox> SizeSlot = SNew(SHorizontalBox);
	CreateNumericField<int32, SHorizontalBox>(SizeSlot, FText::FromString("Width"), ETextJustify::Center, &TextStyle, 0.1f, 0.4f, MIN_WIDTH, OnWidthCommit, OnwidthUpdate);
	CreateNumericField<int32, SHorizontalBox>(SizeSlot, FText::FromString("Height"), ETextJustify::Center, &TextStyle, 0.1f, 0.4f, MIN_HEIGHT, OnHeightCommit, OnHeightUpdate);

	return SizeSlot;
}

TSharedRef<SWidget> FCustomMinesweeperModule::CreateMinesNumSlot()
{
	auto OnMinesNumCommit = [this](int32 InValue, ETextCommit::Type CommitType) -> void { if (CommitType != ETextCommit::Default && ValidateMinesNumber(InValue)) MinesNum = InValue; };
	auto OnMinesNumUpdate = [this]() -> int32 { return MinesNum; };

	TSharedRef<SHorizontalBox> MinesNumSlot = SNew(SHorizontalBox);
	CreateNumericField<int32, SHorizontalBox>(MinesNumSlot, FText::FromString("Mines Number"), ETextJustify::Center, &TextStyle, 0.2f, 0.8f, MIN_MINES_NUM, OnMinesNumCommit, OnMinesNumUpdate);

	return MinesNumSlot;
}

bool FCustomMinesweeperModule::ValidateMinesNumber(int32 InValue)
{
	if (InValue > Width * Height)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Invalid Mines Number"));
		return false;
	}

	return true;
}

TSharedRef<SWidget> FCustomMinesweeperModule::CreateButton(const FText& Name, const FTextBlockStyle* InTextStyle, FReply(FCustomMinesweeperModule::* InFunc)())
{
	return SNew(SButton)
		.Text(Name)
		.TextStyle(InTextStyle)
		.OnClicked_Raw(this, InFunc)
		.VAlign(EVerticalAlignment::VAlign_Center)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.DesiredSizeScale(FVector2D(4.f, 1.f));
}

FReply FCustomMinesweeperModule::CreateGrid()
{
	if (ValidateMinesNumber(MinesNum))
	{
		FillGrid();
		FillGridPanel();
	}	

	return FReply::Handled();
}

void FCustomMinesweeperModule::FillGrid()
{
	Grid.Init(0, Width * Height);
	Left = (Width * Height) - MinesNum;

	SetRandomMines();
}

void FCustomMinesweeperModule::SetRandomMines()
{
	for (int32 Index = 0; Index < MinesNum; Index++)
	{
		int32 RandomX = FMath::RandRange(0, Width - 1);
		int32 RandomY = FMath::RandRange(0, Height - 1);
		Grid[FlattenIndex(RandomX, RandomY)] = MINE_ID;

		SetAround(RandomX, RandomY - 1);
		SetAround(RandomX + 1, RandomY - 1);
		SetAround(RandomX + 1, RandomY);
		SetAround(RandomX + 1, RandomY + 1);
		SetAround(RandomX, RandomY + 1);
		SetAround(RandomX - 1, RandomY + 1);
		SetAround(RandomX - 1, RandomY);
		SetAround(RandomX - 1, RandomY - 1);
	}
}

void FCustomMinesweeperModule::SetAround(int32 X, int32 Y)
{
	if (OutOfRange(X, Y)) return;

	int32 Index = FlattenIndex(X, Y);
	if (Grid[Index] != MINE_ID) Grid[Index]++;
}

int32 FCustomMinesweeperModule::FlattenIndex(int32 X, int32 Y)
{
	return X + (Y * Width);
}

bool FCustomMinesweeperModule::OutOfRange(int32 X, int32 Y)
{
	return (X < 0 || X >= Width || Y < 0 || Y >= Height);
}

void FCustomMinesweeperModule::FillGridPanel()
{
	ClearResources();

	for (int32 Y=0; Y< Height; Y++)
	{
		for (int32 X = 0; X < Width; X++)
		{
			CreateGridSlot(X, Y);
		}
	}
}

void FCustomMinesweeperModule::CreateGridSlot(int32 X, int32 Y)
{
	int Value = Grid[FlattenIndex(X, Y)];

	TSharedRef<SOverlay> Overlay = SNew(SOverlay);	
	TSharedRef<SBox> Box = SNew(SBox).WidthOverride(100).HeightOverride(100);
	Box->SetContent(Overlay);

	if (Value == MINE_ID)
	{
		CreateMineSlot(Overlay, X, Y);
	}
	else
	{
		CreateNumberedSlot(Overlay, X, Y, Value);
	}
	
	UIGrid.Add(Overlay);
	GridPanel->AddSlot(X, Y).Padding(2.f, 2.f).AttachWidget(Box);
}

void FCustomMinesweeperModule::FlagSlot(int32 Index)
{
	auto OnMouseDown = [this, Index](const FGeometry& Geom, const FPointerEvent& PointerEvent)->FReply
		{
			if (PointerEvent.GetEffectingButton().ToString() == FString("RightMouseButton"))
				UIGrid[Index]->RemoveSlot(2);
			return FReply::Handled();
		};

	UIGrid[Index]->AddSlot(2).AttachWidget(SNew(SBorder).BorderImage(&FlagColor)
		.OnMouseButtonDown_Lambda(OnMouseDown));
}

void FCustomMinesweeperModule::CreateMineSlot(TSharedRef<SOverlay> Overlay, int32 X, int32 Y)
{
	int32 Index = FlattenIndex(X, Y);
	auto OnMouseUp = [this](const FGeometry& Geom, const FPointerEvent& PointerEvent) -> FReply { EndGameAction("You Lose."); return FReply::Handled(); };
	auto OnMouseDown = [this, Index](const FGeometry& Geom, const FPointerEvent& PointerEvent) -> FReply
		{
			if (PointerEvent.GetEffectingButton().ToString() == FString("RightMouseButton"))
			{
				FlagSlot(Index);
				return FReply::Handled();
			}
				
			UIGrid[Index]->RemoveSlot(1); 
			return FReply::Handled();
		};

	Overlay->AddSlot(0).AttachWidget(SNew(SBorder).BorderImage(&MineColor)
		.OnMouseButtonUp_Lambda(OnMouseUp));
	
	Overlay->AddSlot(1).AttachWidget(SNew(SBorder).BorderImage(&OverColor)
		.OnMouseButtonDown_Lambda(OnMouseDown).VAlign(EVerticalAlignment::VAlign_Center));
}

void FCustomMinesweeperModule::CreateNumberedSlot(TSharedRef<SOverlay> Overlay, int32 X, int32 Y, int32 Value)
{	
	auto OnMouseUp = [this](const FGeometry& Geom, const FPointerEvent& PointerEvent) -> FReply { if (WinCondition()) EndGameAction("You Win."); return FReply::Handled(); };
	auto OnMouseDown = [this, X, Y](const FGeometry& Geom, const FPointerEvent& PointerEvent) -> FReply
		{
			if (PointerEvent.GetEffectingButton().ToString() == FString("RightMouseButton"))
			{
				FlagSlot(FlattenIndex(X, Y));
				return FReply::Handled();
			}
				
			RecusrsiveShowSlotsAround(X, Y);
			return FReply::Handled();
		};

	TSharedRef<SBorder> Border = SNew(SBorder).BorderImage(&UnderColor)
		.OnMouseButtonUp_Lambda(OnMouseUp).VAlign(EVerticalAlignment::VAlign_Center);
	if (Value)
	{
		Border->SetContent(SNew(STextBlock).Text(FText::FromString(FString::FromInt(Value)))
			.Justification(ETextJustify::Center).TextStyle(&TextStyle));
	}		
	Overlay->AddSlot(0).AttachWidget(Border);	
	
	Overlay->AddSlot(1).AttachWidget(SNew(SBorder).BorderImage(&OverColor)		
		.OnMouseButtonDown_Lambda(OnMouseDown));
}

void FCustomMinesweeperModule::RecusrsiveShowSlotsAround(int32 X, int32 Y)
{
	if (OutOfRange(X, Y)) return;

	int32 Index = FlattenIndex(X, Y);
	int32 Value = Grid[Index];
	if (Value == MINE_ID) return;	

	if (UIGrid[Index]->GetNumWidgets() > 1)
	{
		UIGrid[Index]->RemoveSlot(1);
		Left--;

		if (!Value)
		{
			RecusrsiveShowSlotsAround(X, Y - 1);
			RecusrsiveShowSlotsAround(X + 1, Y - 1);
			RecusrsiveShowSlotsAround(X + 1, Y);
			RecusrsiveShowSlotsAround(X + 1, Y + 1);
			RecusrsiveShowSlotsAround(X, Y + 1);
			RecusrsiveShowSlotsAround(X - 1, Y + 1);
			RecusrsiveShowSlotsAround(X - 1, Y);
			RecusrsiveShowSlotsAround(X - 1, Y - 1);
		}
	}
}

void FCustomMinesweeperModule::EndGameAction(const FString& Msg)
{
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Msg + " Retry?")) == EAppReturnType::Yes)
	{
		CreateGrid();
		return;
	}

	ClearResources();
}

void FCustomMinesweeperModule::ClearResources()
{
	GridPanel->ClearChildren();
	UIGrid.Empty();
}

bool FCustomMinesweeperModule::WinCondition()
{
	return !Left;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomMinesweeperModule, CustomMinesweeper)