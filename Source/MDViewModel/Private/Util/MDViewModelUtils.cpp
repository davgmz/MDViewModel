#include "Util/MDViewModelUtils.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/Class.h"
#include "UObject/Object.h"
#include "ViewModelProviders/MDViewModelProviderBase.h"
#include "WidgetExtensions/MDViewModelWidgetClassExtension.h"

namespace MDViewModelUtils
{
	const FName DefaultViewModelName = TEXT("Default");

	UMDViewModelProviderBase* FindViewModelProvider(const FGameplayTag& ProviderTag)
	{
		if (GEngine != nullptr)
		{
			UMDViewModelProviderBase* const* ProviderPtr = GEngine->GetEngineSubsystemArray<UMDViewModelProviderBase>().FindByPredicate([&ProviderTag](const UMDViewModelProviderBase* Provider)
			{
				return IsValid(Provider) && Provider->GetProviderTag() == ProviderTag;
			});

			if (ProviderPtr != nullptr)
			{
				return *ProviderPtr;
			}
		}

		return nullptr;
	}

	void GetViewModelAssignmentsForWidgetClass(TSubclassOf<UUserWidget> WidgetClass, bool bIncludeAncestorAssignments, TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments)
	{
		if (UWidgetBlueprintGeneratedClass* WBGC = Cast<UWidgetBlueprintGeneratedClass>(WidgetClass))
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 2
			if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>())
#else
			if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>(false))
#endif
			{
				if (bIncludeAncestorAssignments)
				{
					ClassExtension->GetThisAndAncestorAssignments(OutViewModelAssignments);
				}
				else
				{
					OutViewModelAssignments.Append(ClassExtension->GetAssignments());
				}
			}
		}
	}

	void SearchViewModelAssignments(TMap<FMDViewModelAssignment, FMDViewModelAssignmentData>& OutViewModelAssignments, TSubclassOf<UUserWidget> WidgetClass, TSubclassOf<UMDViewModelBase> ViewModelClass, const FGameplayTag& ProviderTag, const FName& ViewModelName)
	{
		if (UWidgetBlueprintGeneratedClass* WBGC = Cast<UWidgetBlueprintGeneratedClass>(WidgetClass))
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 2
			if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>())
#else
			if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>(false))
#endif
			{
				ClassExtension->SearchAssignments(OutViewModelAssignments, ViewModelClass, ProviderTag, ViewModelName);
			}
		}
	}

	bool DoesClassOrSuperClassHaveAssignments(TSubclassOf<UUserWidget> WidgetClass)
	{
		UClass* Class = WidgetClass;
		while (Class != nullptr)
		{
			if (UWidgetBlueprintGeneratedClass* WBGC = Cast<UWidgetBlueprintGeneratedClass>(Class))
			{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 2
				if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>())
#else
				if (const UMDViewModelWidgetClassExtension* ClassExtension = WBGC->GetExtension<UMDViewModelWidgetClassExtension>(false))
#endif
				{
					if (ClassExtension->HasAssignments())
					{
						return true;
					}
				}
			}

			Class = Class->GetSuperClass();
		}

		return false;
	}
}
