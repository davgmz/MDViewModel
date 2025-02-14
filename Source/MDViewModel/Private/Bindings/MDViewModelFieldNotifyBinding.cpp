#include "Bindings/MDViewModelFieldNotifyBinding.h"

#include "ViewModel/MDViewModelBase.h"

void UMDViewModelFieldNotifyBinding::OnViewModelChanged(UMDViewModelBase* OldViewModel, UMDViewModelBase* NewViewModel, int32 EntryIndex, TWeakObjectPtr<UObject> BoundObject) const
{
	if (!ensure(ViewModelFieldNotifyBindings.IsValidIndex(EntryIndex)) || !BoundObject.IsValid())
	{
		return;
	}

	const FMDViewModelFieldNotifyBindingEntry& Entry = ViewModelFieldNotifyBindings[EntryIndex];
	const TTuple<int32, TWeakObjectPtr<UObject>> DelegateKey = { EntryIndex, BoundObject };
	GetTypeHash(DelegateKey);
	if (IsValid(OldViewModel))
	{
		FDelegateHandle* HandlePtr = BoundDelegates.Find(DelegateKey);
		if (HandlePtr != nullptr && HandlePtr->IsValid())
		{
			const UE::FieldNotification::FFieldId FieldId = OldViewModel->GetFieldNotificationDescriptor().GetField(Entry.ViewModelClass, Entry.FieldNotifyName);
			OldViewModel->RemoveFieldValueChangedDelegate(FieldId, *HandlePtr);
			HandlePtr->Reset();
		}
	}

	if (IsValid(NewViewModel))
	{
		const UE::FieldNotification::FFieldId FieldId = NewViewModel->GetFieldNotificationDescriptor().GetField(Entry.ViewModelClass, Entry.FieldNotifyName);
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate = INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UMDViewModelFieldNotifyBinding::OnFieldValueChanged, EntryIndex, BoundObject);
		BoundDelegates.FindOrAdd(DelegateKey) = NewViewModel->AddFieldValueChangedDelegate(FieldId, Delegate);

		// Execute with the currently held value
		OnFieldValueChanged(NewViewModel, FieldId, EntryIndex, BoundObject);
	}
}

void UMDViewModelFieldNotifyBinding::OnFieldValueChanged(UObject* ViewModel, UE::FieldNotification::FFieldId Field, int32 EntryIndex, TWeakObjectPtr<UObject> BoundObject) const
{
	UMDViewModelBase* BoundViewModel = Cast<UMDViewModelBase>(ViewModel);
	if (!ensure(ViewModelFieldNotifyBindings.IsValidIndex(EntryIndex)) || !BoundObject.IsValid() || !IsValid(BoundViewModel))
	{
		return;
	}

	const FMDViewModelFieldNotifyBindingEntry& Entry = ViewModelFieldNotifyBindings[EntryIndex];

	if (UFunction* BoundFunc = BoundObject->FindFunctionChecked(Entry.FunctionNameToBind))
	{
		check(IsValid(BoundFunc) && BoundFunc->NumParms == 1);

		void* ValuePtr = nullptr;
		void* AllocatedParamPtr = nullptr;
		if (const FProperty* Prop = FindFProperty<FProperty>(BoundViewModel->GetClass(), Field.GetName()))
		{
			check(BoundFunc->ParmsSize == Prop->GetSize());
			ValuePtr = Prop->ContainerPtrToValuePtr<void>(BoundViewModel);
		}
		else if (UFunction* Func = FindUField<UFunction>(BoundViewModel->GetClass(), Field.GetName()))
		{
			const FProperty* ReturnProp = Func->GetReturnProperty();
			check(ReturnProp != nullptr && Func->NumParms == 1);

			AllocatedParamPtr = FMemory::Malloc(ReturnProp->GetSize(), ReturnProp->GetMinAlignment());
			ReturnProp->InitializeValue(AllocatedParamPtr);

			check(Func->ParmsSize == ReturnProp->GetSize());
			check(BoundFunc->ParmsSize == ReturnProp->GetSize());

			BoundViewModel->ProcessEvent(Func, AllocatedParamPtr);

			ValuePtr = AllocatedParamPtr;
		}

		if (ValuePtr != nullptr)
		{
			BoundObject->ProcessEvent(BoundFunc, ValuePtr);
		}

		if (AllocatedParamPtr != nullptr)
		{
			FMemory::Free(AllocatedParamPtr);
			AllocatedParamPtr = nullptr;
		}
	}
}
