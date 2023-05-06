#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UWidgetBlueprint;
class UUserWidget;
struct FMDViewModelEditorAssignment;

/**
 *
 */
class MDVIEWMODELEDITOR_API SMDViewModelList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMDViewModelList)
		{
		}

	SLATE_END_ARGS()

	virtual ~SMDViewModelList();

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UWidgetBlueprint* InBlueprint);

private:
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FMDViewModelEditorAssignment> Item, const TSharedRef<STableViewBase>& OwningTable);
	void PopulateAssignments();

	void OnAssignmentsChanged();

	FReply OnAddViewModel();

	void OnEditItem(TSharedPtr<FMDViewModelEditorAssignment> Item);
	void OnDeleteItem(TSharedPtr<FMDViewModelEditorAssignment> Item);

	TSharedPtr<SListView<TSharedPtr<FMDViewModelEditorAssignment>>> AssignmentList;
	TArray<TSharedPtr<FMDViewModelEditorAssignment>> Assignments;
	TSubclassOf<UUserWidget> WidgetClass;
	UWidgetBlueprint* WidgetBP = nullptr;
};
