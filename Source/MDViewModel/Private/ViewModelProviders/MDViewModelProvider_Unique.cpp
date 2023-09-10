#include "ViewModelProviders/MDViewModelProvider_Unique.h"

#include "Engine/Blueprint.h"
#include "Logging/StructuredLog.h"
#include "Util/MDViewModelAssignment.h"
#include "Util/MDViewModelAssignmentData.h"
#include "Util/MDViewModelFunctionLibrary.h"
#include "Util/MDViewModelLog.h"
#include "ViewModel/MDViewModelBase.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_MDVMProvider_Unique, "MDVM.Provider.Unique");


UMDViewModelBase* UMDViewModelProvider_Unique::SetViewModel(IMDViewModelRuntimeInterface& Object, const FMDViewModelAssignment& Assignment, const FMDViewModelAssignmentData& Data)
{
	if (IsValid(Assignment.ViewModelClass))
	{
		UObject* OwningObject = Object.GetOwningObject();
		UE_LOGFMT(LogMDViewModel, Verbose, "Creating Unique View Model with Assignment [{Assignment}] for Object [{ObjectName}]",
			("ObjectName", GetPathNameSafe(OwningObject)),
			("Assignment", Assignment));
		return Object.SetViewModelOfClass(OwningObject, OwningObject, FMDViewModelAssignmentReference(Assignment), Data.ViewModelSettings);
	}

	return nullptr;
}

#if WITH_EDITOR
void UMDViewModelProvider_Unique::GetExpectedContextObjectTypes(const FInstancedStruct& ProviderSettings, const FInstancedStruct& ViewModelSettings, UBlueprint* Blueprint, TArray<TSubclassOf<UObject>>& OutContextObjectClasses) const
{
	if (IsValid(Blueprint))
	{
		OutContextObjectClasses.Add(Blueprint->SkeletonGeneratedClass != nullptr ? Blueprint->SkeletonGeneratedClass : Blueprint->GeneratedClass);
	}
}
#endif
