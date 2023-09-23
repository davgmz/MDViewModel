#include "ViewModelTab/MDViewModelDetails.h"

#include "ViewModelTab/FieldInspector/MDViewModelFieldInspector.h"

void SMDViewModelDetails::Construct(const FArguments& InArgs, const TSharedPtr<FBlueprintEditor>& BlueprintEditor)
{
	const FSlateBrush* HeaderBrushPtr = &FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView").BackgroundBrush;
	ChildSlot
	.Padding(0, 4.f)
	[
		SNew(SSplitter)
		.Orientation(Orient_Horizontal)
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.25f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 0)
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Properties"))
					.ToolTipText(INVTEXT("Blueprint accessible properties and getter functions from the selected view model. Properties marked with FieldNotify can be bound to.\r\nYou can drag and drop these onto into your blueprint graph."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(4.f, 0)
			[
				SAssignNew(PropertyInspector, SMDViewModelFieldInspector, BlueprintEditor)
				.InspectorType(EMDViewModelFieldInspectorType::Properties)
			]
		]
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.25f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 0)
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Events"))
					.ToolTipText(INVTEXT("Blueprint Assignable delegates from the selected view model."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(4.f, 0)
			[
				SAssignNew(EventInspector, SMDViewModelFieldInspector, BlueprintEditor)
				.InspectorType(EMDViewModelFieldInspectorType::Events)
			]
		]
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.25f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 0)
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Commands"))
					.ToolTipText(INVTEXT("Blueprint Callable functions from the selected view model that are not Pure or Static functions.\r\nYou can drag and drop these onto into your blueprint graph."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(4.f, 0)
			[
				SAssignNew(CommandInspector, SMDViewModelFieldInspector, BlueprintEditor)
				.InspectorType(EMDViewModelFieldInspectorType::Commands)
			]
		]
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.25f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f, 0)
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Helpers"))
					.ToolTipText(INVTEXT("Pure Blueprint Callable functions from the selected view model that are not Property Getter functions.\r\nYou can drag and drop these onto into your blueprint graph."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(4.f, 0)
			[
				SAssignNew(HelperInspector, SMDViewModelFieldInspector, BlueprintEditor)
				.InspectorType(EMDViewModelFieldInspectorType::Helpers)
			]
		]
	];

	UpdateViewModel(InArgs._ViewModelClass, InArgs._DebugViewModel, InArgs._ViewModelName);
}

void SMDViewModelDetails::UpdateViewModel(TSubclassOf<UMDViewModelBase> ViewModelClass, UMDViewModelBase* DebugViewModel, const FName& ViewModelName)
{
	if (PropertyInspector.IsValid())
	{
		PropertyInspector->SetReferences(ViewModelClass, DebugViewModel, ViewModelName);
	}

	if (EventInspector.IsValid())
	{
		EventInspector->SetReferences(ViewModelClass, DebugViewModel, ViewModelName);
	}

	if (CommandInspector.IsValid())
	{
		CommandInspector->SetReferences(ViewModelClass, DebugViewModel, ViewModelName);
	}

	if (HelperInspector.IsValid())
	{
		HelperInspector->SetReferences(ViewModelClass, DebugViewModel, ViewModelName);
	}
}
