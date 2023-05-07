#include "ViewModelTab/MDViewModelDetails.h"

#include "Brushes/SlateColorBrush.h"
#include "ViewModelTab/MDViewModelFieldInspector.h"

void SMDViewModelDetails::Construct(const FArguments& InArgs)
{
	const FSlateBrush* HeaderBrushPtr = &FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView").BackgroundBrush;
	ChildSlot
	.Padding(4.f)
	[
		SNew(SSplitter)
		.Orientation(Orient_Horizontal)
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.34f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Properties"))
					.ToolTipText(INVTEXT("Blueprint accessible properties from the selected view model. Properties marked with FieldNotify can be bound to."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(PropertyInspector, SMDViewModelFieldInspector)
				.bIncludeBlueprintVisibleProperties(true)
				.bIncludeBlueprintAssignableProperties(false)
				.bIncludeBlueprintCallable(false)
				.bIncludeBlueprintPure(false)
				.bIncludeFieldNotifyFunctions(true)
			]
		]
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.33f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
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
			[
				SAssignNew(EventInspector, SMDViewModelFieldInspector)
				.bIncludeBlueprintVisibleProperties(false)
				.bIncludeBlueprintAssignableProperties(true)
				.bIncludeBlueprintCallable(false)
				.bIncludeBlueprintPure(false)
				.bIncludeFieldNotifyFunctions(false)
			]
		]
		+SSplitter::Slot()
		.MinSize(300.f)
		.Value(0.33f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(HeaderBrushPtr)
				[
					SNew(STextBlock)
					.Text(INVTEXT("View Model Commands"))
					.ToolTipText(INVTEXT("Blueprint Callable functions from the selected view model that are not Pure functions."))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(CommandInspector, SMDViewModelFieldInspector)
				.bIncludeBlueprintVisibleProperties(false)
				.bIncludeBlueprintAssignableProperties(false)
				.bIncludeBlueprintCallable(true)
				.bIncludeBlueprintPure(false)
				.bIncludeFieldNotifyFunctions(false)
			]
		]
	];

	UpdateViewModel(InArgs._ViewModelClass, InArgs._DebugViewModel);
}

void SMDViewModelDetails::UpdateViewModel(TSubclassOf<UMDViewModelBase> ViewModelClass, UMDViewModelBase* DebugViewModel)
{
	if (PropertyInspector.IsValid())
	{
		PropertyInspector->SetReferences(ViewModelClass, DebugViewModel);
	}

	if (EventInspector.IsValid())
	{
		EventInspector->SetReferences(ViewModelClass, DebugViewModel);
	}

	if (CommandInspector.IsValid())
	{
		CommandInspector->SetReferences(ViewModelClass, DebugViewModel);
	}
}
