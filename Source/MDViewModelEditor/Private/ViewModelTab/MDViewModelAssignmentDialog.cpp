#include "ViewModelTab/MDViewModelAssignmentDialog.h"

#include "BlueprintExtensions/MDViewModelAssignableInterface.h"
#include "Engine/Engine.h"
#include "Customizations/MDViewModelAssignmentEditorObjectCustomization.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "InstancedStructDetails.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "SPrimaryButton.h"
#include "Util/MDViewModelGraphStatics.h"
#include "ViewModel/MDViewModelBase.h"
#include "ViewModelProviders/MDViewModelProviderBase.h"
#include "ViewModelProviders/MDViewModelProvider_Cached.h"
#include "ViewModelTab/MDViewModelAssignmentEditorObject.h"
#include "Widgets/Layout/SUniformGridPanel.h"

TWeakPtr<SWindow> SMDViewModelAssignmentDialog::ActiveDialogWindowPtr;

class FMDVMExpanderDataDetails : public FInstancedStructDataDetails
{
public:
	FMDVMExpanderDataDetails(const TSharedPtr<IPropertyHandle>& InStructProperty)
		: FInstancedStructDataDetails(InStructProperty)
	{
	}
	
	virtual void OnChildRowAdded(IDetailPropertyRow& ChildRow) override
	{
		ChildRow.ShouldAutoExpand(true);

		SetupCachedLifetimeEditCondition(ChildRow);
	}

private:
	void SetupCachedLifetimeEditCondition(IDetailPropertyRow& ChildRow) const
	{
		static const FName EditConditionKey = TEXT("EditConditionLifetime");
		const TSharedPtr<IPropertyHandle> Property = ChildRow.GetPropertyHandle();
		if (!Property.IsValid() || !Property->HasMetaData(EditConditionKey))
		{
			return;
		}
		
		const FString& EditConditions = Property->GetMetaData(EditConditionKey);
		TArray<FString> EditConditionsList;
		EditConditions.ParseIntoArray(EditConditionsList, TEXT(","));

		FGameplayTagContainer EditConditionTags;
		for (const FString& EditConditionString : EditConditionsList)
		{
			const FString CleanEditConditionString = EditConditionString.TrimStartAndEnd();
			if (!CleanEditConditionString.StartsWith(TEXT("MDVM.Provider.Cached.Lifetimes.")))
			{
				continue;
			}
			
			const FGameplayTag EditConditionTag = FGameplayTag::RequestGameplayTag(*CleanEditConditionString);
			if (!EditConditionTag.IsValid())
			{
				continue;
			}

			EditConditionTags.AddTag(EditConditionTag);
		}
		
		TArray<UObject*> Objects;
		Property->GetOuterObjects(Objects);
		if (Objects.IsEmpty())
		{
			return;
		}

		const TWeakObjectPtr<const UMDViewModelAssignmentEditorObject> EditorObjectPtr = Cast<UMDViewModelAssignmentEditorObject>(Objects[0]);
		if (!EditorObjectPtr.IsValid() || EditorObjectPtr->ViewModelProvider != TAG_MDVMProvider_Cached)
		{
			return;
		}

		const TAttribute<bool> EditConditionAttribute = TAttribute<bool>::Create([EditConditionTags, EditorObjectPtr]()
		{
			const UMDViewModelAssignmentEditorObject* EditorObject = EditorObjectPtr.Get();
			if (EditorObject == nullptr)
			{
				return false;
			}

			const FMDViewModelProvider_Cached_Settings* SettingsPtr = EditorObject->ProviderSettings.GetPtr<FMDViewModelProvider_Cached_Settings>();
			if (SettingsPtr == nullptr)
			{
				return false;
			}

			return EditConditionTags.HasTagExact(SettingsPtr->GetLifetimeTag());
		});
		
		const TAttribute<EVisibility> VisibilityAttribute = TAttribute<EVisibility>::Create([EditConditionAttribute]()
		{
			return EditConditionAttribute.Get(false) ? EVisibility::Visible : EVisibility::Collapsed;
		});
		
		ChildRow
			.EditCondition(EditConditionAttribute, nullptr)
			.Visibility(VisibilityAttribute);
	}
};

class FMDVMExpanderDetails : public FInstancedStructDetails
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FMDVMExpanderDetails>();
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		const TSharedRef<FMDVMExpanderDataDetails> DataDetails = MakeShared<FMDVMExpanderDataDetails>(PropertyHandle);
		ChildBuilder.AddCustomBuilder(DataDetails);
	}
};


void SMDViewModelAssignmentDialog::Construct(const FArguments& InArgs, const TSharedRef<SWindow>& InParentWindow)
{
	Mode = InArgs._Mode;
	BlueprintPtr = InArgs._Blueprint;
	EditorItem = InArgs._EditorItem;

	ParentWindow = InParentWindow;
	EditorObject = TStrongObjectPtr<UMDViewModelAssignmentEditorObject>(NewObject<UMDViewModelAssignmentEditorObject>());

	if (const UBlueprint* Blueprint = GetBlueprint())
	{
		EditorObject->BPSkeletonClass = Blueprint->SkeletonGeneratedClass;
	}

	if (EditorItem.IsValid())
	{
		EditorObject->PopulateFromAssignment(*EditorItem.Get(), GetBlueprint());
		
		if (Mode == EMDVMDialogMode::Edit)
		{
			OriginalAssignmentName = EditorObject->ViewModelInstanceName;
		}
	}

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ENameAreaSettings::HideNameArea;
	DetailsViewArgs.bShowScrollBar = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	const TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->RegisterInstancedCustomPropertyTypeLayout(TEXT("InstancedStruct"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMDVMExpanderDetails::MakeInstance));
	TSharedRef<SMDViewModelAssignmentDialog> SharedThis = StaticCastSharedRef<SMDViewModelAssignmentDialog>(AsShared());
	DetailsView->RegisterInstancedCustomPropertyLayout(UMDViewModelAssignmentEditorObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FMDViewModelAssignmentEditorObjectCustomization::MakeInstance, SharedThis));
	DetailsView->SetObject(EditorObject.Get());

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Fill)
		[
			DetailsView
		]
		+SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		.Padding(8)
		[
			SNew(STextBlock)
			.Text(this, &SMDViewModelAssignmentDialog::GetAssignmentError)
			.Visibility(this, &SMDViewModelAssignmentDialog::GetAssignmentErrorVisibility)
		]
		+SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		.Padding(8)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
			+SUniformGridPanel::Slot(0,0)
			[
				SNew(SPrimaryButton)
				.Text(INVTEXT("Add View Model"))
				.Icon(FAppStyle::Get().GetBrush(TEXT("EditableComboBox.Add")))
				.Visibility(this, &SMDViewModelAssignmentDialog::GetAddVisibility)
				.OnClicked(this, &SMDViewModelAssignmentDialog::OnAddClicked)
			]
			+SUniformGridPanel::Slot(0,0)
			[
				SNew(SPrimaryButton)
				.Text(INVTEXT("Save Changes"))
				.Visibility(this, &SMDViewModelAssignmentDialog::GetSaveVisibility)
				.OnClicked(this, &SMDViewModelAssignmentDialog::OnSaveClicked)
			]
			+SUniformGridPanel::Slot(1,0)
			[
				SNew(SButton)
				.Text(INVTEXT("Cancel"))
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([WeakWindow = TWeakPtr<SWindow>(ParentWindow)]()
				{
					if (const TSharedPtr<SWindow> ParentWindow = WeakWindow.Pin())
					{
						ParentWindow->RequestDestroyWindow();
					}

					return FReply::Handled();
				})
			]
		]
	];
}

UBlueprint* SMDViewModelAssignmentDialog::GetBlueprint() const
{
	return BlueprintPtr.Get();
}

void SMDViewModelAssignmentDialog::OpenAssignmentDialog(UBlueprint* Blueprint)
{
	return OpenDialog_Internal(Blueprint);
}

void SMDViewModelAssignmentDialog::OpenEditDialog(UBlueprint* Blueprint, TSharedPtr<FMDViewModelEditorAssignment> EditorItem)
{
	return OpenDialog_Internal(Blueprint, EditorItem);
}

void SMDViewModelAssignmentDialog::OpenDuplicateDialog(UBlueprint* Blueprint, TSharedPtr<FMDViewModelEditorAssignment> EditorItem)
{
	return OpenDialog_Internal(Blueprint, EditorItem, true);
}

TSharedPtr<SMDViewModelAssignmentDialog> SMDViewModelAssignmentDialog::GetActiveDialog()
{
	if (const TSharedPtr<SWindow> ActiveDialogWindow = ActiveDialogWindowPtr.Pin())
	{
		return StaticCastSharedRef<SMDViewModelAssignmentDialog>(ActiveDialogWindow->GetContent());
	}

	return nullptr;
}

void SMDViewModelAssignmentDialog::OpenDialog_Internal(UBlueprint* Blueprint, TSharedPtr<FMDViewModelEditorAssignment> EditorItem, bool bDuplicateItem)
{
	if (const TSharedPtr<SWindow> ActiveDialogWindow = ActiveDialogWindowPtr.Pin())
	{
		// Only one instance allowed at a time
		ActiveDialogWindow->FlashWindow();
		return;
	}
	
	const bool bIsEditMode = EditorItem.IsValid() && !bDuplicateItem;
	const TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(bIsEditMode ? INVTEXT("Edit a View Model Assignment") : INVTEXT("Add a View Model Assignment"))
		.SizingRule( ESizingRule::UserSized )
		.ClientSize( FVector2D( 700.f, 600.f ))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	ActiveDialogWindowPtr = PickerWindow;
	PickerWindow->GetOnWindowClosedEvent().AddStatic(&SMDViewModelAssignmentDialog::OnDialogClosed);

	const TSharedRef<SMDViewModelAssignmentDialog> AssignmentDialog = SNew(SMDViewModelAssignmentDialog, PickerWindow)
		.Blueprint(Blueprint)
		.EditorItem(EditorItem)
		.Mode(bIsEditMode ? EMDVMDialogMode::Edit : (bDuplicateItem ? EMDVMDialogMode::Duplicate : EMDVMDialogMode::Add));

	PickerWindow->SetContent(AssignmentDialog);

	if (FSlateApplication::Get().GetActiveModalWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(PickerWindow, FSlateApplication::Get().GetActiveModalWindow().ToSharedRef());
	}
	else if (FGlobalTabmanager::Get()->GetRootWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(PickerWindow, FGlobalTabmanager::Get()->GetRootWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(PickerWindow);
	}
}

void SMDViewModelAssignmentDialog::OnDialogClosed(const TSharedRef<SWindow>& Window)
{
	if (Window == ActiveDialogWindowPtr)
	{
		ActiveDialogWindowPtr.Reset();
	}
}

EVisibility SMDViewModelAssignmentDialog::GetAddVisibility() const
{
	if (Mode == EMDVMDialogMode::Edit || !EditorObject.IsValid())
	{
		return EVisibility::Collapsed;
	}

	if (EditorObject->ViewModelInstanceName == NAME_None || EditorObject->ViewModelClass == nullptr)
	{
		return EVisibility::Collapsed;
	}

	if (DoesAssignmentHaveError() || !EditorObject->ViewModelProvider.IsValid())
	{
		return EVisibility::Collapsed;
	}

	const UMDViewModelProviderBase* Provider = MDViewModelUtils::FindViewModelProvider(EditorObject->ViewModelProvider);
	TArray<FText> UnusedIssues;
	if (!Provider->ValidateProviderSettings(EditorObject->ProviderSettings, GetBlueprint(), EditorObject->CreateAssignment().Assignment, UnusedIssues))
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

EVisibility SMDViewModelAssignmentDialog::GetSaveVisibility() const
{
	if (Mode != EMDVMDialogMode::Edit || !EditorObject.IsValid())
	{
		return EVisibility::Collapsed;
	}

	if (EditorObject->ViewModelInstanceName == NAME_None || EditorObject->ViewModelClass == nullptr)
	{
		return EVisibility::Collapsed;
	}

	if (DoesAssignmentHaveError() || !EditorObject->ViewModelProvider.IsValid())
	{
		return EVisibility::Collapsed;
	}

	const UMDViewModelProviderBase* Provider = MDViewModelUtils::FindViewModelProvider(EditorObject->ViewModelProvider);
	TArray<FText> UnusedIssues;
	if (!IsValid(Provider) || !Provider->ValidateProviderSettings(EditorObject->ProviderSettings, GetBlueprint(), EditorObject->CreateAssignment().Assignment, UnusedIssues))
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

EVisibility SMDViewModelAssignmentDialog::GetAssignmentErrorVisibility() const
{
	return DoesAssignmentHaveError() ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SMDViewModelAssignmentDialog::OnAddClicked() const
{
	if (IMDViewModelAssignableInterface* Extension = FMDViewModelGraphStatics::GetOrCreateAssignableInterface(GetBlueprint()))
	{
		FScopedTransaction Transaction = FScopedTransaction(INVTEXT("Added View Model Assignment"));
		Extension->AddAssignment(EditorObject->CreateAssignment());
		OnAssignmentAdded.Broadcast(EditorObject->CreateAssignment());
	}

	if (ParentWindow.IsValid())
	{
		ParentWindow->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SMDViewModelAssignmentDialog::OnSaveClicked() const
{
	if (EditorItem.IsValid())
	{
		if (IMDViewModelAssignableInterface* Extension = FMDViewModelGraphStatics::GetOrCreateAssignableInterface(GetBlueprint()))
		{
			FScopedTransaction Transaction = FScopedTransaction(INVTEXT("Updated View Model Assignment"));
			Extension->UpdateAssignment(*EditorItem.Get(), EditorObject->CreateAssignment());
		}
	}

	if (ParentWindow.IsValid())
	{
		ParentWindow->RequestDestroyWindow();
	}

	return FReply::Handled();
}

bool SMDViewModelAssignmentDialog::DoesAssignmentHaveError() const
{
	return !GetAssignmentError().IsEmptyOrWhitespace();
}

FText SMDViewModelAssignmentDialog::GetAssignmentError() const
{
	if (!EditorObject.IsValid())
	{
		return INVTEXT("Internal error, close and try again.");
	}
	
	if (!EditorObject->ViewModelProvider.IsValid())
	{
		return INVTEXT("Select a View Model Provider.");
	}

	if (EditorObject->ViewModelClass == nullptr)
	{
		return INVTEXT("Select a View Model Class.");
	}

	if (EditorObject->ViewModelInstanceName.IsNone())
	{
		return INVTEXT("Enter a valid View Model Name.");
	}

	if (Mode != EMDVMDialogMode::Edit || !OriginalAssignmentName.IsSet() || EditorObject->ViewModelInstanceName != OriginalAssignmentName.GetValue())
	{
		if (FMDViewModelGraphStatics::DoesBlueprintContainViewModelAssignments(GetBlueprint(), EditorObject->ViewModelClass, FGameplayTag::EmptyTag, EditorObject->ViewModelInstanceName))
		{
			return INVTEXT("The assignment's Name and Class pairing is not unique.");
		}
	}

	return FText::GetEmpty();
}
