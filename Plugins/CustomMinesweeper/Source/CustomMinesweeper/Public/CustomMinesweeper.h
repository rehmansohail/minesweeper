// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Brushes/SlateColorBrush.h"
#include "CoreMinimal.h"
#include <functional>
#include "Modules/ModuleManager.h"

#define MIN_WIDTH 2
#define MIN_HEIGHT 2
#define MIN_MINES_NUM 1
#define MINE_ID -1

const FSlateColorBrush OverColor = FSlateColorBrush(FColor(0, 155, 0, 255));
const FSlateColorBrush UnderColor = FSlateColorBrush(FColor(212, 191, 125, 255));
const FSlateColorBrush MineColor = FSlateColorBrush(FColor(255, 0, 0, 255));
const FSlateColorBrush FlagColor = FSlateColorBrush(FColor(0, 127, 255, 255));
const FSlateColor TextColor = FSlateColor(FLinearColor::Black);
const int32 FontSize = 20;

class SGridPanel;

class FCustomMinesweeperModule : public IModuleInterface
{
protected:
	int32 Width;
	int32 Height;
	int32 MinesNum;
	int32 Left;
	FTextBlockStyle TextStyle;

	TArray<int32> Grid;
	TArray<TSharedPtr<SOverlay>> UIGrid;
	TSharedPtr<SGridPanel> GridPanel;

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	TSharedRef<SDockTab> CreateMinesweeperWindow(const FSpawnTabArgs& TabArgs);

protected:	
	void Init();
	TSharedRef<SWidget> CreateSizeSlot();
	TSharedRef<SWidget> CreateMinesNumSlot();
	bool ValidateMinesNumber(int32 InValue);
	TSharedRef<SWidget> CreateButton(const FText& Name, const FTextBlockStyle* InTextStyle, FReply(FCustomMinesweeperModule::* InFunc)());
	FReply CreateGrid();
	void FillGrid();
	void SetRandomMines();
	void SetAround(int32 X, int32 Y);
	int32 FlattenIndex(int32 X, int32 Y);
	bool OutOfRange(int32 X, int32 Y);
	void FillGridPanel();
	void CreateGridSlot(int32 X, int32 Y);
	void FlagSlot(int32 Index);
	void CreateMineSlot(TSharedRef<SOverlay> Overlay, int32 X, int32 Y);
	void CreateNumberedSlot(TSharedRef<SOverlay> Overlay, int32 X, int32 Y, int32 Value);
	void RecusrsiveShowSlotsAround(int32 X, int32 Y);
	void EndGameAction(const FString& Msg);
	void ClearResources();	
	bool WinCondition();

	template<typename T, typename U>
	void CreateNumericField(TSharedRef<U> Box, const FText& Name, 
		ETextJustify::Type NameJustification, const FTextBlockStyle* InTextStyle,
		float FillWidthName, float FillWidthValue, T MinValue,
		const std::function<void(T InValue, ETextCommit::Type CommitType)>& ValueCommittedLambda,
		const std::function<T()>& ValueUpdateLambda)
	{
		Box->AddSlot().FillWidth(FillWidthName).VAlign(EVerticalAlignment::VAlign_Center)
			.AttachWidget(SNew(STextBlock).Text(Name).Justification(NameJustification)
			.Font(InTextStyle->Font));
		Box->AddSlot().FillWidth(FillWidthValue)
			.AttachWidget(SNew(SNumericEntryBox<T>)
				.MinValue(MinValue)
				.OnValueCommitted_Lambda(ValueCommittedLambda)
				.Value_Lambda(ValueUpdateLambda));
	}
};
