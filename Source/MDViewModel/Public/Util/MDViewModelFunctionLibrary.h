#pragma once

#include "CoreMinimal.h"
#include "MDViewModelUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "MDViewModelFunctionLibrary.generated.h"

class UMDViewModelBase;
class UUserWidget;

/**
 *
 */
UCLASS()
class MDVIEWMODEL_API UMDViewModelFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "View Model", meta = (DefaultToSelf = "Widget"))
	static UMDViewModelBase* SetViewModel(UUserWidget* Widget, UMDViewModelBase* ViewModel, FName ViewModelName = TEXT("Default"));

	UFUNCTION(BlueprintCallable, Category = "View Model", meta = (DefaultToSelf = "Widget", DeterminesOutputType="ViewModelClass"))
	static UMDViewModelBase* SetViewModelOfClass(UUserWidget* Widget, TSubclassOf<UMDViewModelBase> ViewModelClass, FName ViewModelName = TEXT("Default"));

	template<typename T>
	static T* SetViewModelOfClass(UUserWidget* Widget, FName ViewModelName = MDViewModelUtils::DefaultViewModelName, TSubclassOf<UMDViewModelBase> ViewModelClass = T::StaticClass());

	UFUNCTION(BlueprintCallable, Category = "View Model", meta = (DefaultToSelf = "Widget", DeterminesOutputType="ViewModelClass"))
	static UMDViewModelBase* GetViewModel(UUserWidget* Widget, TSubclassOf<UMDViewModelBase> ViewModelClass, FName ViewModelName = TEXT("Default"));

	UFUNCTION(BlueprintCallable, Category = "View Model", meta = (DefaultToSelf = "Widget"))
	static bool DoesWidgetHaveViewModelClassAssigned(const UUserWidget* Widget, TSubclassOf<UMDViewModelBase> ViewModelClass, bool bIncludeChildClasses = true);

	template<typename T>
	static T* GetViewModel(UUserWidget* Widget, FName ViewModelName = MDViewModelUtils::DefaultViewModelName, TSubclassOf<UMDViewModelBase> ViewModelClass = T::StaticClass());
};

template <typename T>
T* UMDViewModelFunctionLibrary::SetViewModelOfClass(UUserWidget* Widget, FName ViewModelName, TSubclassOf<UMDViewModelBase> ViewModelClass)
{
	static_assert(TIsDerivedFrom<T, UMDViewModelBase>::Value, "ViewModels must derive from UMDViewModelBase");
	return Cast<T>(SetViewModelOfClass(Widget, ViewModelClass, ViewModelName));
}

template <typename T>
T* UMDViewModelFunctionLibrary::GetViewModel(UUserWidget* Widget, FName ViewModelName, TSubclassOf<UMDViewModelBase> ViewModelClass)
{
	static_assert(TIsDerivedFrom<T, UMDViewModelBase>::Value, "ViewModels must derive from UMDViewModelBase");
	return Cast<T>(GetViewModel(Widget, ViewModelClass, ViewModelName));
}
